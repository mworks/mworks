addpath(getenv('MW_MATLAB_DIR'))

suite = testsuite('mworkstests', 'IncludeSubpackages', true);
runner = matlab.unittest.TestRunner.withTextOutput('Verbosity', 1);
results = runner.run(suite);

numPassed = length(find([results.Passed]));
numFailed = length(find([results.Failed]));
numIncomplete = length(find([results.Incomplete]));
totalTime = sum([results.Duration]);

fprintf("\nResults:\n")
fprintf("  %d passed, %d failed, %d incomplete\n", ...
        numPassed, numFailed, numIncomplete)
fprintf("  %g seconds testing time\n", totalTime)

pause(1)  % Give non-main threads time to finish
quit(numFailed)
