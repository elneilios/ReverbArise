& {
    $ErrorActionPreference = 'Stop'
    try {
        $notePatch = Join-Path $PSScriptRoot 'host-fx-note.vcv'
        $raw = & tar -xOf $notePatch './patch.json'
        if ($LASTEXITCODE -ne 0) { throw "Could not read Note Gate fixture ($LASTEXITCODE)" }
        $baselineJson = $raw -join [Environment]::NewLine
        $bundle = Join-Path $PSScriptRoot 'build-capture/host_fx_input_probe_artefacts/Release/VST3/ReverbArise Host-FX Capture Spike.vst3'

        foreach ($musical in @($false, $true)) {
            $patch = $baselineJson | ConvertFrom-Json -AsHashtable
            $hostFx = @($patch.modules | Where-Object model -eq 'Host-FX')[0]
            $hostFx.data.pluginName = 'ReverbArise Host-FX Capture Spike'
            $hostFx.data.pluginPath = $bundle.Replace('\','/')
            $hostFx.data.Remove('pluginState')
            $patch.path = ''
            if (!$musical) {
                $gate = $patch.modules | Where-Object id -eq 1
                ($gate.params | Where-Object id -eq 2).value = -2.0
            } else {
                $patch.modules = @($patch.modules | Where-Object id -ne 2)
                $sequence = @(0.0, 3.0, 7.0, 10.0, 12.0, 10.0, 7.0, 3.0)
                $steps = for ($step = 0; $step -lt $sequence.Count; ++$step) {
                    @{ id=4+$step; value=$sequence[$step]/12.0 }
                }
                $patch.modules += @(
                    @{ id=4; plugin='Fundamental'; model='SEQ3'; version='2.6.4'; pos=@(12,0);
                       params=@(@{id=0;value=1.0}, @{id=3;value=8}) + @($steps) },
                    @{ id=5; plugin='Fundamental'; model='VCO'; version='2.6.4'; pos=@(40,0);
                       params=@(@{id=2;value=-12}) }
                )
                $hostFx.pos = @(51,0)
                $patch.cables = @($patch.cables | Where-Object inputId -eq 0)
                $patch.cables += @(
                    @{ id=200; outputModuleId=1; outputId=3; inputModuleId=4; inputId=2; color='#ffb437' },
                    @{ id=201; outputModuleId=4; outputId=1; inputModuleId=5; inputId=0; color='#0c8e15' },
                    @{ id=202; outputModuleId=5; outputId=0; inputModuleId=3; inputId=16; color='#0986ad' },
                    @{ id=203; outputModuleId=5; outputId=1; inputModuleId=3; inputId=17; color='#0986ad' }
                )
            }
            $name = if ($musical) { 'capture-synth-phrase' } else { 'capture-boundaries' }
            $payload = Join-Path $PSScriptRoot "$name-fixture"
            New-Item -ItemType Directory -Force -Path $payload | Out-Null
            $patch | ConvertTo-Json -Depth 40 | Set-Content -LiteralPath (Join-Path $payload 'patch.json') -Encoding utf8NoBOM
            $destination = Join-Path $PSScriptRoot "$name.vcv"
            & tar -c --zstd -f $destination -C $payload patch.json
            if ($LASTEXITCODE -ne 0) { throw "Could not write capture patch ($LASTEXITCODE)" }
            Write-Output $destination
        }
    } catch {
        Write-Error -Message $_.Exception.Message -ErrorAction Continue
        exit 1
    }
}
