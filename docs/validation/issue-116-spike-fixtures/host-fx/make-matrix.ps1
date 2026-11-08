param([switch]$SweepPulsePhase, [switch]$IncludeNotes)

& {
    $ErrorActionPreference = 'Stop'
    try {
        $baseline = Join-Path $PSScriptRoot 'host-fx-input.vcv'
        $members = & tar -tf $baseline
        if ($LASTEXITCODE -ne 0) { throw "Could not list baseline archive ($LASTEXITCODE)" }
        $patchMember = $members | Where-Object { $_ -match '(^|/)patch\.json$' } | Select-Object -First 1
        $patchJson = & tar -xOf $baseline $patchMember
        if ($LASTEXITCODE -ne 0) { throw "Could not read baseline patch ($LASTEXITCODE)" }
        $fixture = ($patchJson -join [Environment]::NewLine) | ConvertFrom-Json -AsHashtable
        $loadedHost = @($fixture.modules | Where-Object model -eq 'Host-FX')[0]
        if ($null -eq $loadedHost.data.pluginPath -or @($fixture.cables).Count -ne 4) {
            throw 'The saved baseline must contain the loaded probe and its four cables.'
        }

        $fixture.path = ''
        $fixture.modules = @($fixture.modules | Where-Object plugin -eq 'Fundamental')
        if ($SweepPulsePhase) {
            $pulseLfo = $fixture.modules | Where-Object id -eq 2
            ($pulseLfo.params | Where-Object id -eq 2).value = [math]::Log(9.97, 2)
        }
        $fixture.modules += [ordered]@{
            id = 9; plugin = 'Fundamental'; model = 'Gates'; version = '2.6.4'; pos = @(28, 0)
        }
        $fixture.cables = @(
            [ordered]@{ id=1; outputModuleId=1; outputId=3; inputModuleId=9; inputId=1; color='#ffb437' }
        )
        $colors = @('#c91847', '#0c8e15', '#ffb437', '#0986ad')
        $cableId = 2
        $cases = @(
            @{ id=10; size=16; x=40; pulseModule=2; pulsePort=3 },
            @{ id=11; size=128; x=51; pulseModule=2; pulsePort=3 },
            @{ id=12; size=512; x=62; pulseModule=2; pulsePort=3 },
            @{ id=13; size=256; x=73; pulseModule=9; pulsePort=0 }
        )
        if ($IncludeNotes) {
            $noteArchive = Join-Path $PSScriptRoot 'host-fx-note.vcv'
            $noteJson = & tar -xOf $noteArchive './patch.json'
            if ($LASTEXITCODE -ne 0) { throw "Could not read Note Gate patch ($LASTEXITCODE)" }
            $noteFixture = ($noteJson -join [Environment]::NewLine) | ConvertFrom-Json -AsHashtable
            $noteHost = @($noteFixture.modules | Where-Object model -eq 'Host-FX')[0]
            if ($noteHost.data.parameterIds[0] -eq $loadedHost.data.parameterIds[0] -or $noteHost.data.parameterIds[1] -eq -1) {
                throw 'The Note Gate patch must contain both note mappings.'
            }
            $noteHost.data.pluginPath = $noteHost.data.pluginPath.Replace('/build/', '/build-midi/')
            if (!(Test-Path -LiteralPath $noteHost.data.pluginPath)) { throw 'The Note Probe bundle is missing.' }
            $cases += @(
                @{ id=14; size=256; x=84; pulseModule=2; pulsePort=3 },
                @{ id=15; size=256; x=95; pulseModule=2; pulsePort=3; note=$true },
                @{ id=16; size=512; x=106; pulseModule=2; pulsePort=3; note=$true },
                @{ id=17; size=256; x=117; pulseModule=9; pulsePort=0; note=$true }
            )
        }
        foreach ($case in $cases) {
            $sourceHost = if ($case.note) { $noteHost } else { $loadedHost }
            $hostInstance = ($sourceHost | ConvertTo-Json -Depth 40) | ConvertFrom-Json -AsHashtable
            $hostInstance.id = $case.id
            $hostInstance.pos = @($case.x, 0)
            $hostInstance.data.blockSize = $case.size
            $fixture.modules += $hostInstance

            $endpoints = @(
                @{ source=1; port=3; input=0 },
                @{ source=$case.pulseModule; port=$case.pulsePort; input=1 },
                @{ source=1; port=3; input=16 },
                @{ source=$case.pulseModule; port=$case.pulsePort; input=17 }
            )
            foreach ($endpoint in $endpoints) {
                $fixture.cables += [ordered]@{
                    id=$cableId; outputModuleId=$endpoint.source; outputId=$endpoint.port
                    inputModuleId=$case.id; inputId=$endpoint.input; color=$colors[($cableId - 2) % 4]
                }
                ++$cableId
            }
        }
        $archiveDirectory = Join-Path $PSScriptRoot 'matrix-fixture'
        New-Item -ItemType Directory -Force -Path $archiveDirectory | Out-Null
        $fixture | ConvertTo-Json -Depth 40 | Set-Content -LiteralPath (Join-Path $archiveDirectory 'patch.json') -Encoding utf8NoBOM
        $archiveName = if ($IncludeNotes) { 'host-fx-comparison.vcv' } elseif ($SweepPulsePhase) { 'host-fx-matrix-phase-sweep.vcv' } else { 'host-fx-matrix.vcv' }
        $archivePath = Join-Path $PSScriptRoot $archiveName
        & tar -c --zstd -f $archivePath -C $archiveDirectory patch.json
        if ($LASTEXITCODE -ne 0) { throw "Could not write matrix patch ($LASTEXITCODE)" }
        Write-Output $archivePath
    } catch {
        Write-Error -Message $_.Exception.Message -ErrorAction Continue
        exit 1
    }
}
