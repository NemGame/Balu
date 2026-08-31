$currentPath = Get-Location
$exePath = Join-Path $currentPath "..\..\src\main.exe"
Unblock-File -Path $exePath