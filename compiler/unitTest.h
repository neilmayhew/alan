/*======================================================================*\

  unitTest.h

  Test harness for unit tests in Alan compiler

\*======================================================================*/

#define UNIT

#include <stdio.h>

static int passed = 0;
static int failed = 0;

static void unitFail(char sourceFile[], int lineNumber, char function[])
{
  printf("%s:%d: unit test '%s()' failed!\n", sourceFile, lineNumber, function);
  failed++;
}


static void unitReportProgress(failed, passed)
{
  return;
  printf("failed: %d, passed: %d\n", failed, passed);
}


#define ASSERT(x) (unitAssert((x), __FILE__, __LINE__, __FUNCTION__))

/* Assert a particular test */
void unitAssert(int x, char sourceFile[], int lineNumber, char function[])
{
  (x)? passed++ : unitFail(sourceFile, lineNumber, function);
  unitReportProgress(failed, passed);
}


/* Run the tests in the test case array*/
static void unitTest(void)
{
  Case *current;

  for (current = caseList; current != NULL; current = current->next) {
    (*current->theCase)();
  }
  if (failed == 0)
    printf("All %d unit tests PASSED!!\n", passed);
  else {
    printf("******************************\n");
    printf("%d of %d unit tests FAILED!!\n", failed, passed+failed);
    printf("******************************\n");
  }
}
