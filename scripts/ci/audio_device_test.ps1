# Copyright (C) 2026 SIPez LLC.  All rights reserved.
#
# Create a process-owned virtual audio devnode with swdevice_audio and
# exercise it with wavelock_probe. Both are built from
# sipXmediaLib/src/test/mp by the viability workflow.
#
#   -Mode list     create the device, wait for WinMM to list a capture
#                  device, print the lists, destroy the device.
#                  Exit 1 if WinMM never saw one.
#   -Mode removal  create the device, open capture on it with the probe,
#                  destroy the device after -Offset seconds of live
#                  capture, report whether waveInReset returned.
#
# The device is destroyed by signalling the named event swdevice_audio
# waits on, so the moment of SwDeviceClose is under script control.

param(
  [Parameter(Mandatory=$true)][ValidateSet('list','removal')][string]$Mode,
  [Parameter(Mandatory=$true)][string]$Tools,
  [Parameter(Mandatory=$true)][string]$Hwid,
  [string]$CaptureMatch = '',
  [int]$Offset = 5,
  [string]$LogDir = 'audio-logs'
)

$ErrorActionPreference = 'Continue'
$eventName = 'Global\sipx_swdev_kill'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
function Find-Tool($name) {
  $hit = Get-ChildItem -Path . -Recurse -File -Filter $name -ErrorAction SilentlyContinue |
         Select-Object -First 1
  if (-not $hit) { throw "$name not found under $(Get-Location)" }
  Write-Host "using $($hit.FullName)"
  return $hit.FullName
}
$swdev = Find-Tool 'swdevice_audio.exe'
$probe = Find-Tool 'wavelock_probe.exe'
$swLog = Join-Path $LogDir "swdevice_$Mode.log"

function Signal-Kill {
  try {
    $e = [System.Threading.EventWaitHandle]::OpenExisting($eventName)
    [void]$e.Set()
    $e.Close()
  }
  catch {
    Write-Host "could not open $eventName : $_"
  }
}

function Show-Devices {
  Get-PnpDevice -Class MEDIA -ErrorAction SilentlyContinue |
    Format-Table FriendlyName, Status, InstanceId -AutoSize | Out-String | Write-Host
  Get-PnpDevice -Class AudioEndpoint -ErrorAction SilentlyContinue |
    Format-Table FriendlyName, Status -AutoSize | Out-String | Write-Host
}

function Show-Log($label, $path) {
  if (Test-Path $path) { Get-Content $path | ForEach-Object { Write-Host "${label}: $_" } }
}

$sw = Start-Process -FilePath $swdev -ArgumentList @('--hwid', $Hwid, '--event', $eventName) `
        -RedirectStandardOutput $swLog -PassThru -NoNewWindow

# Wait up to 30 s for the driver to bind and WinMM to list a capture device.
$count = 0
$list = @()
for ($i = 1; $i -le 30; $i++) {
  Start-Sleep -Seconds 1
  if ($sw.HasExited) { break }
  $list = & $probe --method list 2>&1
  $m = $list | Select-String -Pattern '^WinMM capture devices: (\d+)'
  if ($m) { $count = [int]$m.Matches[0].Groups[1].Value }
  if ($count -gt 0) { break }
}
$list | Set-Content (Join-Path $LogDir "probe_list_$Mode.log")
$list | ForEach-Object { Write-Host $_ }
Write-Host "WinMM capture devices after $i s: $count"
Show-Devices
Show-Log 'swdevice' $swLog

if ($count -eq 0) {
  Write-Host 'RESULT: no WinMM capture device appeared'
  Signal-Kill
  [void]$sw.WaitForExit(30000)
  exit 1
}

if ($Mode -eq 'list') {
  Write-Host "RESULT: WinMM capture devices = $count"
  Signal-Kill
  [void]$sw.WaitForExit(30000)
  Show-Log 'swdevice' $swLog
  exit 0
}

# removal: find the capture index of the new device, default 0.
$index = 0
$inCapture = $true
foreach ($line in $list) {
  if ($line -match '^WinMM render devices') { $inCapture = $false }
  if ($inCapture -and $CaptureMatch -ne '' -and
      $line -match '^\s+\[(\d+)\] "(.*)"' -and $Matches[2] -like "*$CaptureMatch*") {
    $index = [int]$Matches[1]
    break
  }
}
Write-Host "capture index for probe: $index"

$probeLog = Join-Path $LogDir 'probe_removed.log'
$pr = Start-Process -FilePath $probe `
        -ArgumentList @('--dir', 'in', '--method', 'removed', '--device', $index,
                        '--reset-timeout', '60000', '--iterations', '3') `
        -RedirectStandardOutput $probeLog -PassThru -NoNewWindow

Start-Sleep -Seconds $Offset
Write-Host "signalling SwDeviceClose after $Offset s of capture"
Signal-Kill

$probeExited = $pr.WaitForExit(180000)
$swExited = $sw.WaitForExit(60000)
Show-Log 'probe' $probeLog
Show-Log 'swdevice' $swLog
Show-Devices

$reset = Get-Content $probeLog -ErrorAction SilentlyContinue |
  Select-String -Pattern '^\s*reset (returned|WEDGED)'
if ($reset) { Write-Host "RESULT: $($reset.Line.Trim())" }
elseif (-not $probeExited) { Write-Host 'RESULT: probe did not exit within 180 s' }
else { Write-Host 'RESULT: probe exited without a reset line' }
if (-not $swExited) { Write-Host 'RESULT: swdevice_audio did not return from SwDeviceClose within 60 s' }

if (-not $probeExited) { Stop-Process -Id $pr.Id -Force -ErrorAction SilentlyContinue }
if (-not $swExited) { Stop-Process -Id $sw.Id -Force -ErrorAction SilentlyContinue }
exit 0

