[CmdletBinding()]
param([Parameter(Mandatory=$true)][string] $Path)
$ErrorActionPreference='Stop'
$lines=Get-Content -LiteralPath $Path
$footer=@($lines | Where-Object { $_ -match '^# written=' })
$rows=@($lines | Where-Object { $_ -notmatch '^#' -and $_.Length -gt 0 } | ConvertFrom-Csv)
$invariant=[Globalization.CultureInfo]::InvariantCulture
$groups=@($rows | Group-Object reset,run | ForEach-Object {
    $items=@($_.Group)
    $tempos=@($items | Where-Object { ([int]$_.flags -band 8) -ne 0 } | ForEach-Object { [double]::Parse($_.bpm,$invariant) })
    [pscustomobject]@{
        Run=$_.Name; Rows=$items.Count; FirstSample=$items[0].sample; LastSample=$items[-1].sample
        Frames=@($items.frames | Group-Object | ForEach-Object {[pscustomobject]@{Frames=$_.Name;Count=$_.Count}})
        OfflineRows=@($items | Where-Object {([int]$_.flags -band 256) -ne 0}).Count
        PlayingRows=@($items | Where-Object {([int]$_.flags -band 32) -ne 0}).Count
        MissingPpqRows=@($items | Where-Object {([int]$_.flags -band 4) -eq 0}).Count
        MissingTempoRows=@($items | Where-Object {([int]$_.flags -band 8) -eq 0}).Count
        Discontinuities=@($items | Where-Object {([int]$_.flags -band 2048) -ne 0}).Count
        BpmMin=if($tempos.Count){($tempos | Measure-Object -Minimum).Minimum}else{$null}
        BpmMax=if($tempos.Count){($tempos | Measure-Object -Maximum).Maximum}else{$null}
        DistinctBpm=@($tempos | Sort-Object -Unique).Count
    }
})
$complete=$false
if($footer.Count -eq 1 -and $footer[0] -match '^# written=(\d+) omitted=0 io_failed=0 queue_bytes=\d+$'){
    $complete=([uint64]$Matches[1] -eq [uint64]$rows.Count)
}
[pscustomobject]@{Trace=(Resolve-Path -LiteralPath $Path).Path; Rows=$rows.Count; Footer=$footer;
    CompleteWithoutReportedLoss=$complete;
    Limits='Raw boundary observations only; no full-map or intra-block precision guarantee.'; Runs=$groups} | ConvertTo-Json -Depth 8
