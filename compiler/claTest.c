/*======================================================================*\

  claTest.c

\*======================================================================*/

#include "cla.c"
#include "adv.h"
#include "ins_x.h"
#include "emit.h"

#include "unit.h"
#include "unitList.h"


void testCreateClass()
{
  Srcp srcp = {1,2,3};
  IdNode *id = newId(srcp, "claId");
  IdNode *parent = newId(srcp, "parentId");
  Instance *ins;

  /* Create a class with unknown inheritance */
  Class *cla = newClass(&srcp, id, parent, NULL);

  ASSERT(equalSrcp(cla->srcp, srcp));
  ASSERT(equalId(cla->props->id, id));
  ASSERT(equalId(cla->props->parentId, parent));

  symbolizeClasses();
  ASSERT(readEcode() == 310 && readSev() == sevERR);

  /* Add the inheritance id, resymbolize */
  ins = newInstance(&srcp, parent, NULL, NULL);
  symbolizeClasses();
  ASSERT(readEcode() == 350 && readSev() == sevERR);
}


void testGenerateClasses()
{
  Srcp srcp = {12,13,14};
  Class *cla;
  Aaddr addr;
  int firstAdr = AwordSizeOf(ACodeHeader);
  static int NOOFPREDEFEINEDCLASSES = 8;
  static int classSize = AwordSizeOf(ClassEntry);
  int baseAddress = firstAdr + NOOFPREDEFEINEDCLASSES*classSize;

  initAdventure();

  initEmit("unit.a3c");
  symbolizeAdventure();
  addr = generateClasses();
  /* Table should start directly after header */
  ASSERT(addr == firstAdr);
  /* header + PREDEFINED classes + 1 EOF should be generated*/
  ASSERT(nextEmitAddress() == baseAddress + 1);

  initEmit("unit.a3c");
  symbolizeClasses();
  cla = newClass(&srcp, newId(srcp, "aSimpleClass"), NULL, NULL);
  addr = generateClasses();
  ASSERT(addr == firstAdr);	/* Should start at first address after header */
  ASSERT(nextEmitAddress() == baseAddress + classSize + 1);	/* (predefined+1) classes + EOF */
}

void testGenerateEmptyClassEntry()
{
  Properties *props = newProps(NULL,
			       NULL,
			       nulsrcp, NULL,
			       NULL,
			       NULL,
			       NULL,
			       nulsrcp, NULL, NULL, NULL, NULL,
			       NULL, NULL,
			       nulsrcp, NULL,
			       NULL,
			       NULL);
  Class *class = newClass(&nulsrcp, newId(nulsrcp, "aClass"), NULL, props);
  int entryAddress;
  ClassEntry *entry;
  
  initAdventure();
  initEmit("unit.a3c");
  symbolizeAdventure();

  generateCommonPropertiesData(class->props);
  entryAddress = nextEmitAddress();
  generateClassEntry(class);
  finalizeEmit();
  writeHeader(&acodeHeader);
  terminateEmit();

  loadACD("unit.a3c");
  entry = (ClassEntry *) &memory[entryAddress];
  ASSERT(convertFromACD(entry->description) == 0);
  ASSERT(convertFromACD(entry->parent) == 0);
}

void claUnitTests()
{
  registerUnitTest(testCreateClass);
  registerUnitTest(testGenerateClasses);
  registerUnitTest(testGenerateEmptyClassEntry);
}

