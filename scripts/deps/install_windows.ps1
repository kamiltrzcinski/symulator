param(
    [switch]$PrintOnly
)

$ErrorActionPreference = 'Stop'

if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
    Write-Error "winget is required. Install App Installer from Microsoft Store."
}

$commands = @(
    @('winget', 'install', '--id', 'Git.Git', '--exact', '--silent', '--accept-package-agreements', '--accept-source-agreements'),
    @('winget', 'install', '--id', 'Kitware.CMake', '--exact', '--silent', '--accept-package-agreements', '--accept-source-agreements'),
    @('winget', 'install', '--id', 'Ninja-build.Ninja', '--exact', '--silent', '--accept-package-agreements', '--accept-source-agreements'),
    @('winget', 'install', '--id', 'LLVM.LLVM', '--exact', '--silent', '--accept-package-agreements', '--accept-source-agreements'),
    @('winget', 'install', '--id', 'Python.Python.3.12', '--exact', '--silent', '--accept-package-agreements', '--accept-source-agreements'),
    @(
        'winget', 'install', '--id', 'Microsoft.VisualStudio.2022.BuildTools', '--exact',
        '--silent', '--accept-package-agreements', '--accept-source-agreements',
        '--override', '--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
    )
)

foreach ($cmd in $commands) {
    if ($PrintOnly) {
        Write-Host ('[print] ' + ($cmd -join ' '))
    }
    else {
        & $cmd[0] $cmd[1..($cmd.Length - 1)]
    }
}

if (-not $PrintOnly) {
    Write-Host 'Windows dependencies installed.'
}
