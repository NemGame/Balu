$currentCerts = Get-ChildItem -Path Cert:\CurrentUser\My, Cert:\CurrentUser\Root -Recurse | Where-Object {$_.Subject -like "*CN=LocallySignedBalu*"}

# Remove existing certificates with the same subject to avoid conflicts
if ($currentCerts) {
    foreach ($cert in $currentCerts) {
        Remove-Item -Path $cert.PSPath -Force
    }
}

# Create certificate
$subject = "CN=LocallySignedBalu"
$cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject $subject -CertStoreLocation Cert:\CurrentUser\My
Move-Item -Path $cert.PSPath -Destination Cert:\CurrentUser\Root

# Sign
$currentPath = Get-Location
$exePath = Join-Path $currentPath "..\..\src\main.exe"
Set-AuthenticodeSignature -FilePath $exePath -Certificate $cert
