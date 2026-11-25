param([string]$GameExe = "$PSScriptRoot/../x64/Debug/2ship.exe")

$ErrorActionPreference = 'Stop'
$game = Get-Item -LiteralPath $GameExe
$runDir = Join-Path $game.DirectoryName ("rom-prompt-check-" + [guid]::NewGuid())
New-Item -ItemType Directory -Path $runDir | Out-Null
foreach ($name in '2ship.exe', '2ship.o2r', 'zapd-oot.exe', 'assets', 'oot-assets') {
    Copy-Item -LiteralPath (Join-Path $game.DirectoryName $name) -Destination $runDir -Recurse
}
New-Item -ItemType Directory -Path (Join-Path $runDir 'text.txt') | Out-Null
Set-Content -LiteralPath (Join-Path $runDir 'test') -Value 'preserve this file'

Write-Host "Choose Majora's Mask first, then Ocarina of Time. Close the game when the title screen appears."
Write-Host 'Both extractors should show an advancing percentage in the same progress dialog.'
$run = Start-Process -FilePath (Join-Path $runDir '2ship.exe') -WorkingDirectory $runDir -Wait -PassThru
if ($run.ExitCode -ne 0) {
    throw "2Ship exited with code $($run.ExitCode)."
}
if (!(Test-Path -LiteralPath (Join-Path $runDir 'text.txt') -PathType Container) -or
    (Get-Content -LiteralPath (Join-Path $runDir 'test') -Raw).Trim() -ne 'preserve this file') {
    throw 'Startup modified unrelated files in the working directory.'
}
$mm, $oot = Get-Item -LiteralPath (Join-Path $runDir 'mm.o2r'), (Join-Path $runDir 'oot.o2r')
if ($mm.Length -eq 0 -or $oot.Length -eq 0 -or $mm.LastWriteTimeUtc -gt $oot.LastWriteTimeUtc) {
    throw 'Expected a Majora''s Mask archive followed by an Ocarina of Time archive.'
}
Write-Host "Both archives were created in $runDir."
