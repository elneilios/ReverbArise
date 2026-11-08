& {
    $ErrorActionPreference = 'Stop'
    try {
        $clockName = 'Rack Loop Fixture Clock'
        $traceName = 'Rack Loop Timing Trace'
        $clock = (Join-Path $PSScriptRoot "build/rack_clock_artefacts/Release/VST3/$clockName.vst3").Replace('\','/')
        $trace = (Join-Path $PSScriptRoot "build/rack_trace_artefacts/Release/VST3/$traceName.vst3").Replace('\','/')
        foreach ($bundle in @($clock,$trace)) {
            if (!(Test-Path -LiteralPath $bundle -PathType Container)) { throw "Missing $bundle" }
        }
        $payload = Join-Path $PSScriptRoot 'patch.json'

        function New-Host([int] $id, [int[]] $position, [int] $block, [switch] $isClock, [switch] $duplicate) {
            $ids = if ($isClock) { @(-1)*24 } else { @(-485,-487,-483,-482) + @(-1)*12 }
            if ($duplicate) { $ids[4] = -483 }
            $mapping = if ($isClock) { '[[[0]],[[0,1,2,3,4,5,6,7]]]' } else { '[[[0,1]],[[0,1]]]' }
            @{
                id=$id; plugin='VCV-Host'; version='2.1.0'; pos=$position
                model=$(if ($isClock) { 'Host-XL' } else { 'Host-FX' })
                params=@()
                data=@{
                    parameterIds=@($ids); subPluginId=0; blockSize=$block; bypass=$false; worker=$true
                    pluginPath=$(if ($isClock) { $clock } else { $trace })
                    pluginName=$(if ($isClock) { $clockName } else { $traceName })
                    busChannelMappings=($mapping | ConvertFrom-Json -NoEnumerate)
                }
            }
        }
        function New-Cable([int] $id, [int] $source, [int] $output, [int] $dest, [int] $inputPort, [string] $color) {
            @{ id=$id; outputModuleId=$source; outputId=$output; inputModuleId=$dest; inputId=$inputPort; color=$color }
        }

        foreach ($lifecycle in @($false,$true)) {
            $modules = @(
                @{ id=1; plugin='Fundamental'; model='Push'; version='2.6.4'; pos=@(0,0) },
                (New-Host 2 @(6,0) 256 -isClock)
            )
            $cables = @(New-Cable 1 1 0 2 24 '#ffb437')
            $nextCable = 2
            $cases = if ($lifecycle) {
                $modules += @{ id=7; plugin='Fundamental'; model='Push'; version='2.6.4'; pos=@(38,0); data=@{hold=$true} }
                @(@{id=3; pos=@(26,0); size=256; audio=@(1,2); duplicate=$true})
            } else {
                @(
                    @{id=3; pos=@(26,0); size=256; audio=@(0,4)},
                    @{id=4; pos=@(36,0); size=256; audio=@(2,3)},
                    @{id=5; pos=@(26,1); size=512; audio=@(0,4)},
                    @{id=6; pos=@(36,1); size=512; audio=@(2,3)}
                )
            }
            foreach ($case in $cases) {
                $modules += New-Host $case.id $case.pos $case.size -duplicate:([bool]$case.duplicate)
                for ($i=0; $i -lt 4; ++$i) {
                    $source = if ($lifecycle -and $i -eq 0) { 7 } else { 2 }
                    $port = if ($source -eq 7) { 1 } else { $i }
                    $cables += New-Cable $nextCable $source $port $case.id $i @('#c91847','#0c8e15','#ffb437','#0986ad')[$i]
                    ++$nextCable
                }
                if ($case.duplicate) {
                    $cables += New-Cable $nextCable 2 2 $case.id 4 '#ffb437'
                    ++$nextCable
                }
                for ($i=0; $i -lt 2; ++$i) {
                    $source = if ($lifecycle -and $i -eq 0) { 7 } else { 2 }
                    $cables += New-Cable $nextCable $source $case.audio[$i] $case.id (16+$i) '#0986ad'
                    ++$nextCable
                }
            }
            $patch = @{ version='2.6.6'; path=''; zoom=0.8; gridOffset=@(0,0); modules=$modules; cables=$cables }
            $patch | ConvertTo-Json -Depth 30 | Set-Content -LiteralPath $payload -Encoding utf8NoBOM
            $name = if ($lifecycle) { 'loop-lifecycle' } else { 'loop-timing' }
            $file = Join-Path $PSScriptRoot "$name.vcv"
            & tar -c --zstd -f $file -C $PSScriptRoot patch.json
            if ($LASTEXITCODE -ne 0) { throw "Failed to create $file ($LASTEXITCODE)" }
            Write-Output $file
        }
    } catch {
        Write-Error -Message $_.Exception.Message -ErrorAction Continue
        exit 1
    }
}
