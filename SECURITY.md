# Security Policy

## Reporting a Vulnerability

If you believe you have found a security vulnerability in sipXtapi, please
report it privately so it can be triaged and fixed before public disclosure.

- **Preferred:** use GitHub's private vulnerability reporting for this
  repository via
  [**Report a vulnerability**](https://github.com/sipXtapi/sipXtapi/security/advisories/new),
  which opens a private advisory draft visible only to the maintainers and you.
- **Email:** alternatively, contact the maintainers at <dpetrie@sipez.com> or
  <sgodin@sipspectrum.com>.

Please include:

- the affected component (STUN, RTP, RTCP, DNS resolver, etc.) and file(s),
- a description of the issue and its impact,
- steps to reproduce or a proof-of-concept if available,
- any suggested remediation.

We aim to acknowledge new reports within a few business days. Please give us a
reasonable opportunity to release a fix before disclosing the issue publicly.

## Supported Versions

sipXtapi is developed on the `master` branch, and fixes are delivered there.
Security fixes are not backported to older tags; users are encouraged to track
`master` (or the latest release) to receive them.

## Disclosure and Advisories

Fixed vulnerabilities are documented as GitHub Security Advisories (GHSAs) for
this repository, with CVE identifiers assigned through GitHub's CNA. Published
advisories are listed on the
[**Security Advisories**](https://github.com/sipXtapi/sipXtapi/security/advisories)
page and flow into the GitHub Advisory Database so downstream users are
notified.

## Acknowledgments

We thank the following researchers for responsibly reporting security issues:

- **Tristan Madani** &lt;TristanInSec@gmail.com&gt; — a set of pre-authentication
  memory-safety issues in the STUN, RTP, RTCP, and DNS response parsers (heap
  out-of-bounds reads/writes and integer underflows), reported in 2026.
