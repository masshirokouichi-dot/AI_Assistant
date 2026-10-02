param([switch]$Build, [switch]$UseExistingDatabase)
$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$board = Get-Content (Join-Path $PSScriptRoot 'arduino-board.json') -Raw | ConvertFrom-Json
$buildPath = Join-Path $PSScriptRoot 'arduino-build'
if (!$UseExistingDatabase) {
    $cliArgs = @('compile', '--config-file', "$env:USERPROFILE/.arduinoIDE/arduino-cli.yaml", '--fqbn', $board.fqbn, '--build-path', $buildPath)
    if (!$Build) { $cliArgs += '--only-compilation-database' }
    & $board.cliPath @cliArgs $project
    if ($LASTEXITCODE -ne 0) { throw "Arduino CLI failed ($LASTEXITCODE)." }
}

# Arduino builds copies under sketch/. Point IntelliSense at the editable sources.
# Expand GCC response files and relative SDK include prefixes for C/C++ tools.
function Expand-Arguments($arguments) {
    foreach ($argument in $arguments) {
        if ($argument.StartsWith('@')) {
            $contents = Get-Content $argument.Substring(1) -Raw
            if ($contents) {
                $tokens = [regex]::Matches($contents, '(?:[^\s"]|"[^"]*")+') | ForEach-Object { $_.Value.Replace('\"', '"') }
                Expand-Arguments $tokens
            }
        } else { $argument }
    }
}
$database = Get-Content (Join-Path $buildPath 'compile_commands.json') -Raw | ConvertFrom-Json
$mapped = @(foreach ($entry in $database) {
    if (!$entry.file.StartsWith((Join-Path $buildPath 'sketch'), [StringComparison]::OrdinalIgnoreCase)) { continue }
    $name = Split-Path $entry.file -Leaf
    $isSketch = $name.EndsWith('.ino.cpp')
    if ($isSketch) { $name = $name.Substring(0, $name.Length - 4) }
    $source = Join-Path $project $name
    $expanded = @(Expand-Arguments $entry.arguments)
    $prefix = ''
    $normalized = @(for ($i = 0; $i -lt $expanded.Count; $i++) {
        $arg = $expanded[$i]
        if ($arg -eq '-iprefix') { $prefix = $expanded[++$i]; continue }
        if ($arg -eq '-iwithprefixbefore' -or $arg -eq '-iwithprefix') { '-I' + (Join-Path $prefix $expanded[++$i]); continue }
        if ($arg -eq $entry.file) { $source } else { $arg }
    })
    if ($isSketch) { $normalized = @($normalized[0], '-x', 'c++', '-include', 'Arduino.h') + $normalized[1..($normalized.Count - 1)] }
    [ordered]@{ directory = $project; file = $source; arguments = $normalized }
})
$mapped | ConvertTo-Json -Depth 8 | Set-Content (Join-Path $PSScriptRoot 'compile_commands.json') -Encoding UTF8
$sample = $mapped | Where-Object { $_.file.EndsWith('camera.cpp') } | Select-Object -First 1
if (!$sample) { throw 'No project compilation entry found.' }
$compiler = $sample.arguments[0]
if (!(Test-Path $compiler) -and (Test-Path "$compiler.exe")) { $compiler += '.exe' }
$includes = @($sample.arguments | Where-Object { $_.StartsWith('-I') } | ForEach-Object { $_.Substring(2).Replace('\', '/') } | Where-Object { Test-Path $_ })
$defines = @($sample.arguments | Where-Object { $_.StartsWith('-D') } | ForEach-Object { $_.Substring(2) })
$config = [ordered]@{
    configurations = @([ordered]@{
        name = 'ESP32-S3 Arduino'; compilerPath = $compiler.Replace('\', '/');
        compileCommands = '${workspaceFolder}/.vscode/compile_commands.json';
        includePath = $includes; defines = $defines;
        cStandard = 'gnu17'; cppStandard = 'gnu++20'; intelliSenseMode = 'windows-gcc-x64'
    }); version = 4
}
$config | ConvertTo-Json -Depth 8 | Set-Content (Join-Path $PSScriptRoot 'c_cpp_properties.json') -Encoding UTF8
Write-Host "Updated IntelliSense for $($mapped.Count) project source files."
