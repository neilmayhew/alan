/*======================================================================*\

  addTest.c

  Unit tests for ADD node in the Alan compiler

\*======================================================================*/

#include "add.c"

#include "sym.h"

static void testMultipleAddAttribute()
{
  IdNode *theId = newId(&nulsrcp, "aClassId");
  ClaNod *theClass;
  Attribute *theFirstAttribute = newAttribute(&nulsrcp,
					      BOOLEAN_TYPE,
					      newId(&nulsrcp, "firstAttribute"),
					      0, 0, 0);
  Attribute *theSecondAttribute = newAttribute(&nulsrcp,
					       BOOLEAN_TYPE,
					       newId(&nulsrcp, "secondAttribute"),
					       0, 0, 0);
  Properties *theFirstAttributeProps = newProps(NULL, NULL,
						concat(NULL, theFirstAttribute,
						       ATTRIBUTE_LIST),
						NULL,
						nulsrcp, NULL, nulsrcp, NULL,
						NULL, nulsrcp, NULL,
						nulsrcp, NULL, NULL, NULL);
  Properties *theSecondAttributeProps = newProps(NULL, NULL,
						 concat(NULL, theSecondAttribute,
							ATTRIBUTE_LIST),
						 NULL,
						 nulsrcp, NULL, nulsrcp, NULL,
						 NULL, nulsrcp, NULL,
						 nulsrcp, NULL, NULL, NULL);
  AddNode *add1 = newAdd(&nulsrcp, theId, NULL, theFirstAttributeProps);
  AddNode *add2 = newAdd(&nulsrcp, theId, NULL, theSecondAttributeProps);
  Symbol *aSymbol;

  theClass = newClass(&nulsrcp, theId, NULL, NULL);
  aSymbol = lookup("aClassId");
  ASSERT(aSymbol != NULL);
  ASSERT(length(aSymbol->fields.entity.props->attributes) == 0);

  addAttributes(add1, aSymbol);
  ASSERT(length(aSymbol->fields.entity.props->attributes) == 1);
  
  addAttributes(add2, aSymbol);
  ASSERT(length(aSymbol->fields.entity.props->attributes) == 2);
  
}

void registerAddUnitTests()
{
  registerUnitTest(testMultipleAddAttribute);
}

