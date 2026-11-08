& {
    $ErrorActionPreference = 'Stop'
    $clockName = 'Rack Loop Policy Clock'
    $probeName = 'Rack Loop Policy Probe'
    $clock = (Join-Path $PSScriptRoot "build/rack_policy_clock_artefacts/Release/VST3/$clockName.vst3").Replace('\','/')
    $probe = (Join-Path $PSScriptRoot "build/rack_policy_artefacts/Release/VST3/$probeName.vst3").Replace('\','/')
    foreach ($bundle in @($clock,$probe)) {
        if (!(Test-Path -LiteralPath $bundle -PathType Container)) { throw "Missing $bundle" }
    }
    $raw = & tar -xOf (Join-Path $PSScriptRoot 'loop-timing.vcv') 'patch.json'
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $patch = ($raw -join [Environment]::NewLine) | ConvertFrom-Json -AsHashtable
    $patch.modules = @($patch.modules | Where-Object id -in 1,2,3,5)
    $generator = $patch.modules | Where-Object id -eq 2
    $generator.data.pluginPath = $clock
    $generator.data.pluginName = $clockName
    $patch.cables = @($patch.cables | Where-Object inputModuleId -eq 2)
    $cableId = 2
    foreach ($observer in @($patch.modules | Where-Object id -in 3,5)) {
        $observer.data.pluginPath = $probe
        $observer.data.pluginName = $probeName
        $observer.data.parameterIds = @(-487,-483,-482) + @(-1)*13
        foreach ($wire in @(@(1,0,'#0c8e15'),@(6,1,'#ffb437'),@(7,2,'#c91847'),@(4,16,'#0986ad'),@(5,17,'#0986ad'))) {
            $patch.cables += @{ id=$cableId; outputModuleId=2; outputId=$wire[0]
                inputModuleId=$observer.id; inputId=$wire[1]; color=$wire[2] }
            ++$cableId
        }
    }
    $payload = Join-Path $PSScriptRoot 'policy-fixture'
    $null = New-Item -ItemType Directory -Path $payload -Force
    $patch | ConvertTo-Json -Depth 30 | Set-Content -LiteralPath "$payload/patch.json" -Encoding utf8NoBOM
    $destination = Join-Path $PSScriptRoot 'loop-policy.vcv'
    & tar -c --zstd -f $destination -C $payload patch.json
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Output $destination
}
