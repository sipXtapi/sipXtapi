#!/usr/bin/env python3
#
# Copyright (C) 2026 SIPez LLC. .  All rights reservied.
#
# graph_test_results.py
#
# Reads sipX test_summary_*.json files from one or more directories and
# produces a set of HTML pages (one per sipX project) with interactive
# plotly graphs comparing test results across platforms over time.
#
# Each project page has two subplots sharing an x-axis (date):
#   Top:    one line per platform showing the count of test points run.
#   Bottom: stacked areas per platform showing failed / hangs / aborts.
#
# An index.html links to all project pages.
#
# Usage:
#   python3 graph_test_results.py [--input DIR ...] [--output DIR]
#                                 [--recursive] [--min-total-ran N]
#

import argparse
import colorsys
import datetime
import json
import os
import sys
import re
from collections import defaultdict

import plotly.graph_objects as go
import plotly.offline
from plotly.subplots import make_subplots


CANONICAL_PROJECTS = [
    "sipXportLib",
    "sipXsdpLib",
    "sipXtackLib",
    "sipXmediaLib",
    "sipXmediaAdapterLib",
    "sipXcallLib",
    "sipXtapi",
]

DEFAULT_MIN_TOTAL_RAN = 10000

# Predefined color families. Each family has three shades, lightest to
# darkest, used for failed / hangs / aborts respectively. The middle
# shade is also used for the platform's "ran" line on the top subplot.
PLATFORM_COLOR_FAMILIES = [
    # red
    ("rgb(255,200,200)", "rgb(220,80,80)",  "rgb(150,30,30)"),
    # blue
    ("rgb(200,220,255)", "rgb(60,120,200)", "rgb(20,50,130)"),
    # green
    ("rgb(200,235,200)", "rgb(60,160,60)",  "rgb(20,90,20)"),
    # orange
    ("rgb(255,220,180)", "rgb(230,140,40)", "rgb(160,80,10)"),
    # purple
    ("rgb(225,200,240)", "rgb(150,80,180)", "rgb(80,30,110)"),
    # teal
    ("rgb(195,230,230)", "rgb(40,150,150)", "rgb(15,80,80)"),
]


def parse_args():
    p = argparse.ArgumentParser(
        description="Graph sipX unit test results across platforms over time."
    )
    p.add_argument(
        "--input", nargs="+", default=["."],
        help="One or more directories containing test_summary_*.json files."
             " Default: current directory."
    )
    p.add_argument(
        "--output", default="./graphs",
        help="Directory to write the generated HTML files. Default: ./graphs"
    )
    p.add_argument(
        "--recursive", action="store_true",
        help="Recurse into subdirectories of each input directory."
    )
    p.add_argument(
        "--min-total-ran", type=int, default=DEFAULT_MIN_TOTAL_RAN,
        help="Minimum sum of 'ran' across all projects for a run to be"
             " considered complete. Default: %d" % DEFAULT_MIN_TOTAL_RAN
    )
    p.add_argument(
        "--hide-hostname", action="store_true",
        help="Omit hostname from legend labels (still shown on hover)."
             " Useful for CI runners with random hostnames."
    )
    return p.parse_args()


def discover_files(dirs, recursive):
    found = set()
    for d in dirs:
        if not os.path.isdir(d):
            print("Warning: %s is not a directory, skipping" % d,
                  file=sys.stderr)
            continue
        if recursive:
            for root, _subdirs, files in os.walk(d):
                for fn in files:
                    if fn.startswith("test_summary_") and fn.endswith(".json"):
                        found.add(os.path.abspath(os.path.join(root, fn)))
        else:
            for fn in os.listdir(d):
                if fn.startswith("test_summary_") and fn.endswith(".json"):
                    full = os.path.join(d, fn)
                    if os.path.isfile(full):
                        found.add(os.path.abspath(full))
    return sorted(found)


def load_summary(path):
    try:
        with open(path) as f:
            data = json.load(f)
    except (IOError, OSError, ValueError) as e:
        print("Warning: could not read %s: %s" % (path, e), file=sys.stderr)
        return None

    bd = data.get("builddate")
    bt = data.get("buildtime")
    if not bd or not bt:
        print("Warning: %s missing builddate/buildtime, skipping" % path,
              file=sys.stderr)
        return None

    try:
        # builddate YYYYMMDD, buildtime HHMM
        dt = datetime.datetime(
            int(bd[0:4]), int(bd[4:6]), int(bd[6:8]),
            int(bt[0:2]), int(bt[2:4])
        )
    except (ValueError, IndexError) as e:
        print("Warning: %s has bad date/time %r %r: %s"
              % (path, bd, bt, e), file=sys.stderr)
        return None

    data["_datetime"] = dt
    data["_path"] = path
    return data


def total_ran(summary):
    projects = summary.get("projects", {})
    total = 0
    for name in CANONICAL_PROJECTS:
        proj = projects.get(name)
        if isinstance(proj, dict):
            try:
                total += int(proj.get("ran", 0))
            except (TypeError, ValueError):
                pass
    return total


def is_complete_run(summary, min_total_ran):
    return total_ran(summary) >= min_total_ran


TOOLCHAIN_RE = re.compile(r"^([a-z]+-\d+)")

def normalize_toolchain(raw):
    if not raw:
        return "unknown"
    m = TOOLCHAIN_RE.match(raw)
    if not m:
        return "unknown"
    return m.group(1)


def series_label(summary, show_hostname=True):
    distro = summary.get("hostdistro") or "unknown"
    toolchain = normalize_toolchain(summary.get("build_toolchain"))
    if show_hostname:
        host = summary.get("hostname") or "unknown"
        return "%s (%s) [%s]" % (host, distro, toolchain)
    return "%s [%s]" % (distro, toolchain)


def group_by_series(summaries, show_hostname=True):
    groups = defaultdict(list)
    for s in summaries:
        groups[series_label(s, show_hostname)].append(s)
    for label in groups:
        groups[label].sort(key=lambda s: s["_datetime"])
    return dict(groups)


def _hsv_family(hue):
    # Generate a (light, medium, dark) family from a hue in [0,1).
    def rgb(h, s, v):
        r, g, b = colorsys.hsv_to_rgb(h, s, v)
        return "rgb(%d,%d,%d)" % (int(r * 255), int(g * 255), int(b * 255))
    return (rgb(hue, 0.25, 0.98), rgb(hue, 0.65, 0.75), rgb(hue, 0.85, 0.45))


def _rgba(rgb_str, alpha):
    inside = rgb_str[rgb_str.index("(") + 1:rgb_str.rindex(")")]
    return "rgba(%s,%g)" % (inside, alpha)


def _integer_ticks(values, target_count=6):
    if not values:
        return [0, 1]
    vmax = max(values)
    if vmax <= 0:
        return [0, 1]
    if vmax < target_count:
        return list(range(0, vmax + 1))
    step = max(1, vmax // (target_count - 1))
    ticks = list(range(0, vmax + 1, step))
    if ticks[-1] < vmax:
        ticks.append(vmax)
    return ticks


def assign_platform_colors(platform_labels):
    color_map = {}
    labels = sorted(platform_labels)
    for i, label in enumerate(labels):
        if i < len(PLATFORM_COLOR_FAMILIES):
            light, medium, dark = PLATFORM_COLOR_FAMILIES[i]
        else:
            # Place additional platforms on hues that avoid the predefined
            # families. Step around the wheel using the golden ratio.
            extra_index = i - len(PLATFORM_COLOR_FAMILIES)
            hue = (0.07 + extra_index * 0.6180339887) % 1.0
            light, medium, dark = _hsv_family(hue)
        color_map[label] = {
            "ran": medium,
            "failed": light,
            "hangs": medium,
            "aborts": dark,
        }
    return color_map


def _project_values(grouped, project, stat, include_hover=False):
    # Returns dict: platform_label -> (list of x datetimes, list of y values)
    result = {}
    for label, runs in grouped.items():
        xs = []
        ys = []
        hover = []
        for s in runs:
            proj = s.get("projects", {}).get(project)
            if not isinstance(proj, dict):
                xs.append(s["_datetime"])
                ys.append(0)
            else:
                try:
                    ys.append(int(proj.get(stat, 0)))
                except (TypeError, ValueError):
                    ys.append(0)
                xs.append(s["_datetime"])
            if include_hover:
                hover.append([
                    s.get("hostname") or "unknown",
                    s.get("hostdistro") or "unknown",
                    s.get("hostarch") or "unknown",
                    s.get("hostkernel") or "unknown",
                    s.get("build_toolchain") or "unknown",
                ])
        if include_hover:
            result[label] = (xs, ys, hover)
        else:
            result[label] = (xs, ys)
    return result


def build_project_figure(project, grouped, color_map):
    fig = make_subplots(
        rows=2, cols=1,
        shared_xaxes=True,
        vertical_spacing=0.08,
        row_heights=[0.55, 0.45],
        subplot_titles=("Tests run", "Failed / Hangs / Aborts (stacked per platform)"),
    )

    # Top subplot: one line per platform, "ran"
    ran_data = _project_values(grouped, project, "ran", include_hover=True)
    hover_tmpl = (
        "<b>%{customdata[0]}</b><br>"
        "distro: %{customdata[1]}<br>"
        "arch: %{customdata[2]}<br>"
        "kernel: %{customdata[3]}<br>"
        "toolchain: %{customdata[4]}<br>"
        "%{x}<br>"
        "{stat_label}: %{y}<extra></extra>"
    )

    for label in sorted(ran_data.keys()):
        xs, ys, hv = ran_data[label]
        fig.add_trace(
            go.Scatter(
                x=xs, y=ys,
                mode="lines+markers",
                name=label,
                legendgroup=label,
                line=dict(color=color_map[label]["ran"], width=2),
                marker=dict(size=5),
                customdata=hv,
                hovertemplate=hover_tmpl.replace("{stat_label}", "ran"),
            ),
            row=1, col=1,
        )

    # Bottom subplot: per platform, stacked failed -> hangs -> aborts
    failed_data = _project_values(grouped, project, "failed", include_hover=True)
    hangs_data = _project_values(grouped, project, "hangs", include_hover=True)
    aborts_data = _project_values(grouped, project, "aborts", include_hover=True)

    top_values = []
    for label in ran_data:
        top_values.extend(ran_data[label][1])
    bottom_values = []
    for label in failed_data:
        bottom_values.extend(failed_data[label][1])
        bottom_values.extend(hangs_data[label][1])
        bottom_values.extend(aborts_data[label][1])

    for label in sorted(grouped.keys()):
        xs_f, ys_f, hv_f = failed_data[label]
        xs_h, ys_h, hv_h = hangs_data[label]
        xs_a, ys_a, hv_a = aborts_data[label]

        # stackgroup keyed by platform so the three areas stack on each
        # other but different platforms do not stack across each other.
        stackgroup = "stack_" + label
        # legendgroup matches the top subplot so toggling the platform
        # in the legend hides all four traces (ran + failed/hangs/aborts).

        fig.add_trace(
            go.Scatter(
                x=xs_f, y=ys_f,
                mode="lines+markers",
                name=label + " failed",
                legendgroup=label,
                showlegend=False,
                stackgroup=stackgroup,
                line=dict(width=0.5, color=color_map[label]["failed"]),
                fillcolor=_rgba(color_map[label]["failed"], 0.15),
                marker=dict(size=3, color=color_map[label]["failed"], opacity=0.7),
                customdata=hv_f,
                hovertemplate=hover_tmpl.replace("{stat_label}", "failed"),
            ),
            row=2, col=1,
        )
        fig.add_trace(
            go.Scatter(
                x=xs_h, y=ys_h,
                mode="lines+markers",
                name=label + " hangs",
                legendgroup=label,
                showlegend=False,
                stackgroup=stackgroup,
                line=dict(width=0.5, color=color_map[label]["hangs"]),
                fillcolor=_rgba(color_map[label]["hangs"], 0.15),
                marker=dict(size=3, color=color_map[label]["hangs"], opacity=0.7),
                customdata=hv_h,
                hovertemplate=hover_tmpl.replace("{stat_label}", "hangs"),
            ),
            row=2, col=1,
        )
        fig.add_trace(
            go.Scatter(
                x=xs_a, y=ys_a,
                mode="lines+markers",
                name=label + " aborts",
                legendgroup=label,
                showlegend=False,
                stackgroup=stackgroup,
                line=dict(width=0.5, color=color_map[label]["aborts"]),
                fillcolor=_rgba(color_map[label]["aborts"], 0.15),
                marker=dict(size=3, color=color_map[label]["aborts"], opacity=0.7),
                customdata=hv_a,
                hovertemplate=hover_tmpl.replace("{stat_label}", "aborts"),
            ),
            row=2, col=1,
        )

    fig.update_layout(
        title=project + " unit tests",
        hovermode="x unified",
        legend=dict(title="Platform (click to toggle)"),
        height=800,
        hoverlabel=dict(font=dict(size=10)),
    )
    fig.update_xaxes(title_text="date", row=2, col=1)
    fig.update_yaxes(title_text="ran", exponentformat="none",
                     tickformat="d",
                     tickmode="array", tickvals=_integer_ticks(top_values),
                     rangemode="nonnegative", row=1, col=1)
    fig.update_yaxes(title_text="problems", exponentformat="none",
                     tickformat="d",
                     tickmode="array", tickvals=_integer_ticks(bottom_values),
                     rangemode="nonnegative", row=2, col=1)

    return fig


def write_project_html(fig, outdir, project):
    out_path = os.path.join(outdir, project + "_stats.html")
    plotly.offline.plot(
        fig,
        filename=out_path,
        auto_open=False,
        include_plotlyjs=True,
    )
    return out_path


def write_index(outdir, projects, platform_labels, run_count,
                date_range, skipped_count):
    if date_range[0] and date_range[1]:
        date_str = "%s to %s" % (
            date_range[0].strftime("%Y-%m-%d %H:%M"),
            date_range[1].strftime("%Y-%m-%d %H:%M"),
        )
    else:
        date_str = "(no runs)"

    rows = []
    rows.append("<!doctype html>")
    rows.append("<html><head><meta charset='ascii'>")
    rows.append("<title>sipX test results</title>")
    rows.append("<style>")
    rows.append("body{font-family:sans-serif;max-width:900px;margin:2em auto;padding:0 1em}")
    rows.append("h1{border-bottom:1px solid #ccc}")
    rows.append("ul{line-height:1.7}")
    rows.append(".meta{background:#f4f4f4;padding:0.8em 1em;border-radius:4px}")
    rows.append(".meta dt{font-weight:bold;float:left;clear:left;width:10em}")
    rows.append(".meta dd{margin-left:11em}")
    rows.append("</style>")
    rows.append("</head><body>")
    rows.append("<h1>sipX unit test results</h1>")

    rows.append("<div class='meta'><dl>")
    rows.append("<dt>Runs included:</dt><dd>%d</dd>" % run_count)
    rows.append("<dt>Runs skipped:</dt><dd>%d</dd>" % skipped_count)
    rows.append("<dt>Date range:</dt><dd>%s</dd>" % date_str)
    rows.append("<dt>Platforms:</dt><dd>")
    if platform_labels:
        rows.append("<ul style='margin:0'>")
        for p in sorted(platform_labels):
            rows.append("<li>%s</li>" % p)
        rows.append("</ul>")
    else:
        rows.append("(none)")
    rows.append("</dd>")
    rows.append("</dl></div>")

    rows.append("<h2>Project graphs</h2>")
    rows.append("<ul>")
    for p in projects:
        rows.append("<li><a href='%s_stats.html'>%s</a></li>" % (p, p))
    rows.append("</ul>")
    rows.append("<p style='color:#888;font-size:0.85em'>Generated %s</p>"
                % datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
    rows.append("</body></html>")

    out_path = os.path.join(outdir, "index.html")
    with open(out_path, "w") as f:
        f.write("\n".join(rows))
    return out_path


def main():
    args = parse_args()

    paths = discover_files(args.input, args.recursive)
    if not paths:
        print("No test_summary_*.json files found in: %s"
              % ", ".join(args.input), file=sys.stderr)
        return 1

    summaries = []
    for p in paths:
        s = load_summary(p)
        if s is not None:
            summaries.append(s)

    complete = []
    skipped = 0
    for s in summaries:
        if is_complete_run(s, args.min_total_ran):
            complete.append(s)
        else:
            skipped += 1

    if not complete:
        print("No complete runs found (min total ran = %d)."
              " Found %d incomplete file(s)."
              % (args.min_total_ran, skipped), file=sys.stderr)
        return 1

    grouped = group_by_series(complete, show_hostname=not args.hide_hostname)
    color_map = assign_platform_colors(grouped.keys())

    if not os.path.isdir(args.output):
        os.makedirs(args.output)

    for project in CANONICAL_PROJECTS:
        fig = build_project_figure(project, grouped, color_map)
        out_path = write_project_html(fig, args.output, project)
        print("wrote %s" % out_path)

    all_dts = [s["_datetime"] for s in complete]
    date_range = (min(all_dts), max(all_dts))

    index_path = write_index(
        args.output, CANONICAL_PROJECTS, list(grouped.keys()),
        len(complete), date_range, skipped,
    )
    print("wrote %s" % index_path)
    print("Open file://%s in a browser." % os.path.abspath(index_path))
    return 0


if __name__ == "__main__":
    sys.exit(main())

