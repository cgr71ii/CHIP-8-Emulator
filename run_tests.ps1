
$testsDir = "tests"
$testsOutputExt = ".out"
$testsExpectedExt = ".exp"

echo "Utility for run the tests in Windows."
echo "If a test doesn't display nothing, it means that has passed."
echo ""

# Obtain tests executables
$tests = Get-ChildItem $testsDir | where {$_.extension -eq ".exe"}

# Run tests
foreach ($test in $tests)
{
    echo "Running $test ..."

    Invoke-Expression "$testsDir/$test" > "$testsDir/$test$testsOutputExt"
    $expected = Get-ChildItem $testsDir | where {$_.name -eq "$test$testsExpectedExt"}

    if (!$expected)
    {
        echo "Expected file for $test not found..."
        echo ""
    }
    if ($expected)
    {
        # Check if the test has the expected result

        echo "Comparing $test$testsOutputExt and $test$testsExpectedExt ..."
        echo "---------------------------------------------------------------------"
        Compare-Object (cat "$testsDir/$test$testsOutputExt") (cat "$testsDir/$test$testsExpectedExt") | Out-Host
        echo "---------------------------------------------------------------------"
        echo ""
    }
}

pause