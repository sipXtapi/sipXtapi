# Copyright (C) 2026 SIPez LLC.  All rights reserved.
#
# Put a virtual audio driver package into the driver store without
# creating a device. Device creation is done separately by
# swdevice_audio so the devnode's lifetime is owned by a test process.
#
# Writes SIPX_HWID and SIPX_CAPTURE_MATCH to GITHUB_ENV for later steps.

param(
  [Parameter(Mandatory=$true)][ValidateSet('vbcable','vad')][string]$Driver,
  [string]$VbCableUrl = 'https://download.vb-audio.com/Download_CABLE/VBCABLE_Driver_Pack45.zip',
  [string]$VadTag = '25.7.14',
  [string]$VadHwid = 'Root\VirtualAudioDriver'
)

$ErrorActionPreference = 'Stop'

# Server images ship with the audio services disabled.
foreach ($svc in 'AudioEndpointBuilder', 'audiosrv') {
  Set-Service -Name $svc -StartupType Automatic
  Start-Service -Name $svc
}
Get-Service AudioEndpointBuilder, audiosrv | Format-Table Name, Status -AutoSize | Out-String | Write-Host

$work = Join-Path $env:TEMP "sipx_$Driver"
New-Item -ItemType Directory -Force -Path $work | Out-Null
$headers = @{ 'User-Agent' = 'sipxtapi-ci' }

if ($Driver -eq 'vbcable') {
  $zip = Join-Path $work 'vbcable.zip'
  Invoke-WebRequest -Uri $VbCableUrl -OutFile $zip -Headers $headers
  Expand-Archive -Path $zip -DestinationPath $work -Force
  $inf = Get-ChildItem -Path $work -Filter 'vbMmeCable64_win10.inf' -Recurse | Select-Object -First 1
  $hwid = 'VBAudioVACWDM'
  $match = 'CABLE Output'
}
else {
  $api = "https://api.github.com/repos/VirtualDrivers/Virtual-Audio-Driver/releases/tags/$VadTag"
  if ($env:GITHUB_TOKEN) { $headers['Authorization'] = "Bearer $env:GITHUB_TOKEN" }
  $release = Invoke-RestMethod -Uri $api -Headers $headers
  $asset = $release.assets |
    Where-Object { $_.name -match '\.zip$' -and $_.name -match '(signed|x64)' } |
    Select-Object -First 1
  if (-not $asset) {
    $asset = $release.assets | Where-Object { $_.name -match '\.zip$' } | Select-Object -First 1
  }
  if (-not $asset) { throw "no zip asset in VAD release $VadTag" }
  Write-Host "VAD asset: $($asset.name)"
  $zip = Join-Path $work 'vad.zip'
  Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $zip -Headers @{ 'User-Agent' = 'sipxtapi-ci' }
  Expand-Archive -Path $zip -DestinationPath $work -Force
  $inf = Get-ChildItem -Path $work -Filter '*.inf' -Recurse |
    Where-Object { $_.Name -match 'VirtualAudioDriver' } | Select-Object -First 1
  $sys = Get-ChildItem -Path $work -Filter '*.sys' -Recurse | Select-Object -First 1
  if ($sys) {
    $sig = Get-AuthenticodeSignature $sys.FullName
    Write-Host "VAD signature: $($sig.Status) $($sig.SignerCertificate.Subject)"
    if ($sig.SignerCertificate) {
      $store = [System.Security.Cryptography.X509Certificates.X509Store]::new('TrustedPublisher', 'LocalMachine')
      $store.Open('ReadWrite'); $store.Add($sig.SignerCertificate); $store.Close()
    }
  }
  $hwid = $VadHwid
  $match = 'Virtual Audio'
}

if (-not $inf) { throw "INF not found for $Driver under $work" }
Write-Host "INF: $($inf.FullName)"
Write-Host '--- INF sections ---'
Select-String -Path $inf.FullName -Pattern '^\s*\[' | ForEach-Object { Write-Host "  $($_.Line)" }
Write-Host '--- INF model lines (hardware ids) ---'
Select-String -Path $inf.FullName -Pattern '^\s*%.*=.*,' | ForEach-Object { Write-Host "  $($_.Line)" }

# /add-driver only. No /install, so no root devnode is created here and
# VB-Cable's single-instance rule is never hit.
$out = & pnputil /add-driver $inf.FullName 2>&1
$rc = $LASTEXITCODE
$out | ForEach-Object { Write-Host "  $_" }
Write-Host "pnputil exit code: $rc"
if ($rc -ne 0) { throw "pnputil /add-driver failed with $rc" }

Add-Content -Path $env:GITHUB_ENV -Value "SIPX_HWID=$hwid"
Add-Content -Path $env:GITHUB_ENV -Value "SIPX_CAPTURE_MATCH=$match"
Write-Host "hardware id for SwDeviceCreate: $hwid"

