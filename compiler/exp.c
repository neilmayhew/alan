/*----------------------------------------------------------------------*\

                                EXP.C
                           Expression Nodes

\*----------------------------------------------------------------------*/

#include "exp_x.h"

#include "util.h"

#include "srcp_x.h"
#include "whr_x.h"
#include "id_x.h"
#include "atr_x.h"
#include "sym_x.h"
#include "wht_x.h"
#include "cnt_x.h"
#include "lst_x.h"
#include "set_x.h"
#include "context_x.h"

#include "lmList.h"

#include "adv.h"                /* ADV-node */
#include "lst.h"                /* LST-nodes */
#include "elm.h"                /* ELM-nodes */
#include "ins.h"                /* INS-nodes */
#include "opt.h"

#include "emit.h"

#include "../interpreter/acode.h"
#include "encode.h"

#include "dump.h"



/*======================================================================*/
Expression *newExpression(Srcp *srcp, ExpressionKind kind)
{
  Expression *new;                      /* The newly allocated area */

  showProgress();

  new = NEW(Expression);

  new->srcp = *srcp;
  new->kind = kind;
  new->not = FALSE;

  return(new);
}


/*======================================================================*/
Expression *newWhatExpression(Srcp srcp, What *what) {
  Expression *exp = newExpression(&srcp, WHAT_EXPRESSION);
  exp->fields.wht.wht = what;
  return exp;
}


/*======================================================================*/
Expression *newStringExpression(Srcp srcp, long fpos, int len)
{
  Expression *exp = newExpression(&srcp, STRING_EXPRESSION);
  exp->fields.str.fpos = fpos;
  exp->fields.str.len = len;
  return exp;
}

/*======================================================================*/
Expression *newIntegerExpression(Srcp srcp, int value)
{
  Expression *exp = newExpression(&srcp, INTEGER_EXPRESSION);
  exp->fields.val.val = value;
  return exp;
}

/*======================================================================*/
Expression *newAttributeExpression(Srcp srcp, IdNode *attribute, Bool not, Expression *ofWhat) {
  Expression *exp = newExpression(&srcp, ATTRIBUTE_EXPRESSION);
  exp->fields.atr.id = attribute;
  exp->not = not;
  exp->fields.atr.wht = ofWhat;
  return exp;
}

/*======================================================================*/
Expression *newIsaExpression(Srcp srcp, Expression *what, Bool not, IdNode *class) {
  Expression *exp = newExpression(&srcp, ISA_EXPRESSION);
  exp->fields.isa.what = what;
  exp->not = not;
  exp->fields.isa.class = class;
  return exp;
}

/*======================================================================*/
Expression *newAggregateExpression(Srcp srcp, AggregateKind kind,
				   IdNode *attribute, List *filters) {
  Expression *exp = newExpression(&srcp, AGGREGATE_EXPRESSION);
  exp->fields.agr.kind = kind;
  exp->fields.agr.attribute = attribute;
  exp->fields.agr.filters = filters;
  return exp;
}


/*======================================================================*/
void symbolizeExpression(Expression *exp) {
  switch (exp->kind) {
  case WHERE_EXPRESSION:
    symbolizeExpression(exp->fields.whr.wht);
    symbolizeExpression(exp->fields.whr.wht);
    break;
  case WHAT_EXPRESSION:
    symbolizeWhat(exp->fields.wht.wht);
    break;
  default:
    SYSERR("Unexpected Expression kind");
  }
}



/*======================================================================*/
Bool equalTypes(TypeKind typ1, TypeKind typ2)
{
  if (typ1 == UNINITIALIZED_TYPE || typ2 == UNINITIALIZED_TYPE)
    SYSERR("Unintialised type");
  return (typ1 == ERROR_TYPE || typ2 == ERROR_TYPE || typ1 == typ2);
}


/*----------------------------------------------------------------------*/
static Bool expressionIsContainer(Expression *exp, Context *context) {
  return symbolIsContainer(symbolOfExpression(exp, context));
}

/*----------------------------------------------------------------------*/
static void expressionIsNotContainer(Expression *exp, Context *context,
                                     char constructDescription[]) {
  switch (exp->kind) {
  case WHAT_EXPRESSION:
    whatIsNotContainer(exp->fields.wht.wht, context, constructDescription); break;
  case ATTRIBUTE_EXPRESSION:
    lmLogv(&exp->srcp, 311, sevERR, "Expression", "a Container", "because the class of the attribute infered from its initial value does not have the Container property", NULL);
    break;
  default:
    SYSERR("Unexpected Expression kind");
  }
}

/*======================================================================*/
void verifyContainerExpression(Expression *what, Context *context,
                               char constructDescription[]) {

  if (what->type != ERROR_TYPE) {
    if (what->type != INSTANCE_TYPE)
      lmLogv(&what->srcp, 428, sevERR, constructDescription, "an instance", NULL);
    else if (!expressionIsContainer(what, context))
      expressionIsNotContainer(what, context, constructDescription);
  }
}


/*======================================================================*/
Symbol *symbolOfExpression(Expression *exp, Context *context) {
  if (exp == NULL) return NULL;

  switch (exp->kind) {
  case WHAT_EXPRESSION:
    return symbolOfWhat(exp->fields.wht.wht, context);
  case ATTRIBUTE_EXPRESSION:
    return exp->class;
  default:
    SYSERR("Unexpected expression kind");
  }
  return NULL;
}


/*======================================================================*/
Symbol *contentOf(Expression *what, Context *context) {

  /* Find what classes a container takes */

  Symbol *symbol = NULL;
  Symbol *content = NULL;

  switch (what->kind) {
  case WHAT_EXPRESSION:
    symbol = symbolOfWhat(what->fields.wht.wht, context);
    content = contentOfSymbol(symbol);
    break;
  case ATTRIBUTE_EXPRESSION:
    symbol = what->class;
    content = contentOfSymbol(symbol);
    break;
  default:
    SYSERR("Unexpected What kind");
    break;
  }
  return content;
}


/*----------------------------------------------------------------------*/
static char *aggregateToString(AggregateKind agr)
{
  switch (agr) {
  case SUM_AGGREGATE: return("SUM"); break;
  case MAX_AGGREGATE: return("MAX"); break;
  case MIN_AGGREGATE: return("MIN"); break;
  case COUNT_AGGREGATE: return("COUNT"); break;
  default: SYSERR("Unexpected aggregate kind");
  }
  return NULL;
}


/*----------------------------------------------------------------------*/
static void analyzeWhereExpression(Expression *exp, Context *context)
{
  Expression *what = exp->fields.whr.wht;
  Where *where = exp->fields.whr.whr;

  switch (where->kind) {
  case WHERE_HERE:
  case WHERE_NEAR:
    break;
  case WHERE_AT:
    analyzeExpression(where->what, context);
    if (where->what->type != ERROR_TYPE)
      if (where->what->type != INSTANCE_TYPE)
        lmLogv(&where->what->srcp, 428, sevERR, "Expression after AT", "an instance", NULL);
    break;
  case WHERE_IN:
    analyzeExpression(where->what, context);
    if (where->what->type != SET_TYPE) /* Can be in a container and in a set */
      verifyContainerExpression(where->what, context, "Expression after IN");
    break;
  default:
    SYSERR("Unrecognized switch");
    break;
  }

  analyzeExpression(what, context);
  if (what->type != ERROR_TYPE) {
    if (what->type == INSTANCE_TYPE) {
      switch (what->kind) {
      case WHAT_EXPRESSION:
        switch (what->fields.wht.wht->kind) {
        case WHAT_LOCATION:
          lmLogv(&what->srcp, 324, sevERR, "Current Location", "the What-clause of a Where expression", NULL);
          break;
        case WHAT_ID:
        case WHAT_THIS:
        case WHAT_ACTOR:
          break;
        default:
          SYSERR("Unrecognized switch");
          break;
        }
        break;
      case ATTRIBUTE_EXPRESSION:
        break;
      default:
        SYSERR("Unrecognized switch");
        break;
      }
    } else if (where->what->type != SET_TYPE)
      lmLogv(&what->srcp, 428, sevERR, "The What clause of a Where expression", "an instance", NULL);
  }

  if (where->kind == WHERE_IN && where->what->type == SET_TYPE) {
    where->kind = WHERE_INSET;
    verifySetMember(where->what, what, "Set member test");
  }
  exp->type = BOOLEAN_TYPE;
}



/*----------------------------------------------------------------------*/
static TypeKind verifyExpressionAttribute(Expression *attributeExpression,
                                          Attribute *foundAttribute)
{
  IdNode *attributeId = attributeExpression->fields.atr.id;
  TypeKind type = UNINITIALIZED_TYPE;

  if (attributeExpression->kind != ATTRIBUTE_EXPRESSION)
    SYSERR("Not an Attribute Expression");

  if (foundAttribute == NULL) {
    return ERROR_TYPE;
  } else if (foundAttribute->id->symbol == NULL) {
    attributeId->code = foundAttribute->id->code;
    attributeExpression->fields.atr.atr = foundAttribute;
    type = foundAttribute->type;
  } else
    SYSERR("Attribute with symbol");
  return type;
}


/*----------------------------------------------------------------------*/
static void analyzeAttributeExpression(Expression *exp, Context *context)
{
  Attribute *atr;
  Expression *what = exp->fields.atr.wht;

  analyzeExpression(what, context);

  switch (what->kind) {
  case WHAT_EXPRESSION:
    atr = resolveAttribute(what, exp->fields.atr.id, context);
    exp->type = verifyExpressionAttribute(exp, atr);
    if (exp->type == INSTANCE_TYPE) {
      if (atr->instance->symbol != NULL)
	/* Set the expressions class to the class of the attribute */
	exp->class = atr->instance->symbol->fields.entity.parent;
    } else if (exp->type == SET_TYPE)
      exp->class = classOfMembers(exp);
    break;

  case ATTRIBUTE_EXPRESSION:
    if (what->type != ERROR_TYPE) {
      if (what->type != INSTANCE_TYPE) {
        exp->type = ERROR_TYPE;
        lmLogv(&what->srcp, 428, sevERR, "Expression", "an instance", NULL);
      } else {
        atr = resolveAttribute(what, exp->fields.atr.id, context);
        exp->type = verifyExpressionAttribute(exp, atr);
      }
    }
    break;

  default:
    exp->type = ERROR_TYPE;
    lmLog(&exp->srcp, 420, sevERR, "attribute reference");
  }
}

/*----------------------------------------------------------------------*/
static Bool isConstantIdentifier(IdNode *id)
{
  return id->symbol->kind != PARAMETER_SYMBOL
    && id->symbol->kind != LOCAL_SYMBOL;
}


/*----------------------------------------------------------------------*/
static void analyzeBinaryExpression(Expression *exp, Context *context)
{
  analyzeExpression(exp->fields.bin.left, context);
  analyzeExpression(exp->fields.bin.right, context);

  switch (exp->fields.bin.op) {
  case AND_OPERATOR:
  case OR_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, BOOLEAN_TYPE))
      lmLogv(&exp->fields.bin.left->srcp, 330, sevERR, "boolean", "AND/OR", NULL);
    if (!equalTypes(exp->fields.bin.right->type, BOOLEAN_TYPE))
      lmLogv(&exp->fields.bin.right->srcp, 330, sevERR, "boolean", "AND/OR", NULL);
    exp->type = BOOLEAN_TYPE;
    break;

  case NE_OPERATOR:
  case EQ_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, exp->fields.bin.right->type))
      lmLog(&exp->srcp, 331, sevERR, "expression");
    else if (exp->fields.bin.left->type != ERROR_TYPE && exp->fields.bin.right->type != ERROR_TYPE)
      if (exp->fields.bin.left->type == INSTANCE_TYPE) {
        What *leftWhat = exp->fields.bin.left->fields.wht.wht;
        What *rightWhat = exp->fields.bin.right->fields.wht.wht;
        if (leftWhat->kind == WHAT_ID && rightWhat->kind == WHAT_ID)
          if (isConstantIdentifier(leftWhat->id)
              && isConstantIdentifier(rightWhat->id))
            lmLog(&exp->srcp, 417, sevINF, NULL);
      }
    exp->type = BOOLEAN_TYPE;
    break;

  case EXACT_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, STRING_TYPE))
      lmLogv(&exp->fields.bin.left->srcp, 330, sevERR, "string", "'=='", NULL);
    if (!equalTypes(exp->fields.bin.right->type, STRING_TYPE))
      lmLogv(&exp->fields.bin.right->srcp, 330, sevERR, "string", "'=='", NULL);
    break;
            
  case LE_OPERATOR:
  case GE_OPERATOR:
  case LT_OPERATOR:
  case GT_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, INTEGER_TYPE))
      lmLogv(&exp->fields.bin.left->srcp, 330, sevERR, "integer", "relational", NULL);
    if (!equalTypes(exp->fields.bin.right->type, INTEGER_TYPE))
      lmLogv(&exp->fields.bin.right->srcp, 330, sevERR, "integer", "relational", NULL);
    exp->type = BOOLEAN_TYPE;
    break;

  case PLUS_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, INTEGER_TYPE)
        && !equalTypes(exp->fields.bin.left->type, STRING_TYPE))
      lmLogv(&exp->fields.bin.left->srcp, 330, sevERR, "integer or string", "arithmetic", NULL);
    if (!equalTypes(exp->fields.bin.right->type, INTEGER_TYPE)
        && !equalTypes(exp->fields.bin.right->type, STRING_TYPE))
      lmLogv(&exp->fields.bin.right->srcp, 330, sevERR, "integer or string", "arithmetic", NULL);
    if (!equalTypes(exp->fields.bin.left->type, exp->fields.bin.right->type)) {
      lmLog(&exp->srcp, 331, sevERR, "expression");
      exp->type = ERROR_TYPE;
    } else
      exp->type = exp->fields.bin.left->type;
    break;

  case MINUS_OPERATOR:
  case MULT_OPERATOR:
  case DIV_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, INTEGER_TYPE))
      lmLogv(&exp->fields.bin.left->srcp, 330, sevERR, "integer", "arithmetic", NULL);
    if (!equalTypes(exp->fields.bin.right->type, INTEGER_TYPE))
      lmLogv(&exp->fields.bin.right->srcp, 330, sevERR, "integer", "arithmetic", NULL);
    exp->type = INTEGER_TYPE;
    break;

  case CONTAINS_OPERATOR:
    if (!equalTypes(exp->fields.bin.left->type, STRING_TYPE))
      lmLogv(&exp->fields.bin.left->srcp, 330, sevERR, "string", "'CONTAINS'", NULL);
    if (!equalTypes(exp->fields.bin.right->type, STRING_TYPE))
      lmLogv(&exp->fields.bin.right->srcp, 330, sevERR, "string", "'CONTAINS'", NULL);
    exp->type = BOOLEAN_TYPE;
    break;

  default:
    SYSERR("Unrecognized binary operator");
    break;    
  }
}


/*----------------------------------------------------------------------*/
static void analyzeAttributeFilter(Expression *theFilterExpression,
				   IdNode *classId,
				   char *aggregateString)
{
  Attribute *attribute;
  IdNode *attributeId = theFilterExpression->fields.atr.id;

  if (classId != NULL && classId->symbol != NULL) {
    /* Only do attribute semantic check if class is defined */
    attribute = findAttribute(classId->symbol->fields.entity.props->attributes,
                        attributeId);
    if (attribute == NULL) {
      lmLogv(&attributeId->srcp, 316, sevERR, attributeId->string,
             "instances aggregated over using",
             aggregateString,
             classId->symbol->string, NULL);
    } else if (!equalTypes(BOOLEAN_TYPE, attribute->type)) {
      lmLog(&attributeId->srcp, 440, sevERR, "Aggregate");
    } else
      attributeId->code = attribute->id->code;
  }
}


/*----------------------------------------------------------------------*/
static IdNode *analyzeClassingFilter(char *message,
				     Context *context,
				     Expression *theFilterExpression,
				     Bool *foundIsa)
{
  IdNode *classId = NULL;

  switch (theFilterExpression->kind) {
  case ISA_EXPRESSION:
    if (*foundIsa)
      lmLogv(&theFilterExpression->srcp, 224, sevERR, "Isa (class)",
	     message, NULL);
    *foundIsa = TRUE;
    classId = theFilterExpression->fields.isa.class;
    (void) symcheck(classId, CLASS_SYMBOL, context);
    break;
  case WHERE_EXPRESSION:
  case ATTRIBUTE_EXPRESSION:
    break;
  default:
    SYSERR("Unimplemented aggregate filter type");
  }
  return classId;
}


/*----------------------------------------------------------------------*/
static Bool analyzeNonClassingFilter(char *message,
				     Context *context,
				     Expression *theFilter,
				     IdNode *classId,
				     Bool *foundWhere)
{
  switch (theFilter->kind) {
  case WHERE_EXPRESSION:
    if (*foundWhere)
      lmLogv(&theFilter->srcp, 224, sevERR, "Where", message, NULL);
    *foundWhere = TRUE;
    analyzeWhere(theFilter->fields.whr.whr, context);
    break;
  case ATTRIBUTE_EXPRESSION:
    if (classId == NULL)
      /* Can not find attributes on all instances, requires ISA filter */
      lmLog(&theFilter->srcp, 226, sevERR, "");
    else
      analyzeAttributeFilter(theFilter, classId, message);
    break;
  case ISA_EXPRESSION:
    break;
  default:
    SYSERR("Unimplemented aggregate filter expression type");
  }
  return TRUE;
}


/*======================================================================*/
IdNode *analyzeFilterExpressions(char *message, Expression *exp,
				 Context *context, IdNode *classId) {
  List *lst;
  Bool foundWhere = FALSE;
  Bool foundIsa = FALSE;

  /* Pick up the first ISA_EXPRESSION as this will be used to analyze
     the availability of attributes */
  TRAVERSE(lst, exp->fields.agr.filters) {
    IdNode *foundClassId = analyzeClassingFilter(message, context,
						 lst->element.exp, &foundIsa);
    if (foundClassId)
      classId = foundClassId;
  }

  TRAVERSE(lst, exp->fields.agr.filters) {
    if (!analyzeNonClassingFilter(message,  context, lst->element.exp,
				  classId, &foundWhere))
      exp->type = ERROR_TYPE;
  }

  return(classId);
}


/*----------------------------------------------------------------------*/
static void analyzeAggregate(Expression *exp, Context *context)
{
  Attribute *atr = NULL;
  IdNode *classId = NULL;       /* Identifier for class filter if any */
  char message[200] = "";
  exp->type = INTEGER_TYPE;

  strcat(message, aggregateToString(exp->fields.agr.kind));
  strcat(message, " Aggregation");

  classId = analyzeFilterExpressions(message, exp, context, classId);
  if (classId == NULL)
    lmLog(&exp->srcp, 225, sevWAR, aggregateToString(exp->fields.agr.kind));

  if (exp->fields.agr.kind != COUNT_AGGREGATE) {
    /* Now analyze the attribute to do the arithmetic aggregation on */
    if (!classId)
      /* Absence of a class filter makes arithmetic aggregates impossible since
         attributes can never be guaranteed to be defined in all instances. */
      lmLog(&exp->fields.agr.attribute->srcp, 226, sevERR, "");
    else if (classId->symbol != NULL) {
      /* Only do this if there was a correct class filter found */
      atr = findAttribute(classId->symbol->fields.entity.props->attributes,
                          exp->fields.agr.attribute);
      if (atr == NULL) {
        lmLogv(&exp->fields.agr.attribute->srcp, 316, sevERR,
               exp->fields.agr.attribute->string,
               "instances aggregated over using",
               aggregateToString(exp->fields.agr.kind),
               classId->symbol->string, NULL);
      } else if (!equalTypes(INTEGER_TYPE, atr->type)) {
        lmLog(&exp->fields.agr.attribute->srcp, 418, sevERR, "");
      } else
        exp->fields.agr.attribute->code = atr->id->code;
    }
  }
}


/*----------------------------------------------------------------------*/
static void analyzeRandom(Expression *exp, Context *context)
{
  exp->type = INTEGER_TYPE;
  analyzeExpression(exp->fields.rnd.from, context);
  if (!equalTypes(INTEGER_TYPE, exp->fields.rnd.from->type)) {
    lmLogv(&exp->fields.rnd.from->srcp, 408, sevERR, "Expression", "RANDOM expression", "integer", NULL);
    exp->type = ERROR_TYPE;
  }

  analyzeExpression(exp->fields.rnd.to, context);
  if (!equalTypes(INTEGER_TYPE, exp->fields.rnd.to->type)) {
    lmLogv(&exp->fields.rnd.to->srcp, 408, sevERR, "Expression", "RANDOM expression", "integer", NULL);
    exp->type = ERROR_TYPE;
  }
}


/*----------------------------------------------------------------------*/
static void analyzeWhatExpression(Expression *exp, Context *context)
{
  Symbol *symbol;
  IdNode *classId;

  if (exp->kind != WHAT_EXPRESSION)
    SYSERR("Not a WHAT-expression");
  if (!verifyWhatContext(exp->fields.wht.wht, context)) {
    exp->type = ERROR_TYPE;
    return;
  }

  switch (exp->fields.wht.wht->kind) {

  case WHAT_LOCATION:
    exp->type = INSTANCE_TYPE;
    exp->class = locationSymbol;
    break;

  case WHAT_ACTOR:
    if (context->kind == EVENT_CONTEXT)
      lmLog(&exp->fields.wht.wht->srcp, 412, sevERR, "");
    exp->type = INSTANCE_TYPE;
    exp->class = actorSymbol;
    break;

  case WHAT_ID:
    symbol = symcheck(exp->fields.wht.wht->id, INSTANCE_SYMBOL, context);
    if (symbol != NULL) {
      switch (symbol->kind) {
      case PARAMETER_SYMBOL:
        exp->type = symbol->fields.parameter.type;
        exp->class = symbol->fields.parameter.class;
        break;
      case LOCAL_SYMBOL:
        exp->type = symbol->fields.local.type;
        break;
      case INSTANCE_SYMBOL:
        exp->type = INSTANCE_TYPE;
        exp->class = symbol->fields.entity.parent;
        break;
      default:
        SYSERR("Unexpected symbolKind");
        break;
      }
    } else
      exp->type = ERROR_TYPE;
    break;

  case WHAT_THIS:
    exp->type = INSTANCE_TYPE;
    classId = classIdInContext(context);
    if (classId)
      exp->class = classId->symbol; 
    break;

  default:
    SYSERR("Unrecognized switch");
    break;
  }
}


/*----------------------------------------------------------------------*/
static void analyzeBetweenExpression(Expression *exp, Context *context)
{
  analyzeExpression(exp->fields.btw.val, context);
  if (!equalTypes(exp->fields.btw.val->type, INTEGER_TYPE))
    lmLogv(&exp->fields.btw.val->srcp, 330, sevERR, "integer", "'BETWEEN'", NULL);

  analyzeExpression(exp->fields.btw.low, context);
  if (!equalTypes(exp->fields.btw.low->type, INTEGER_TYPE))
    lmLogv(&exp->fields.btw.low->srcp, 330, sevERR, "integer", "'BETWEEN'", NULL);

  analyzeExpression(exp->fields.btw.high, context);
  if (!equalTypes(exp->fields.btw.high->type, INTEGER_TYPE))
    lmLogv(&exp->fields.btw.high->srcp, 330, sevERR, "integer", "'BETWEEN'", NULL);

  exp->type = BOOLEAN_TYPE;
}


/*----------------------------------------------------------------------*/
static void analyzeIsaExpression(Expression *expression,
                                 Context *context)
{
  switch (expression->fields.isa.what->kind) {
  case WHAT_EXPRESSION:
    switch (expression->fields.isa.what->fields.wht.wht->kind) {
    case WHAT_ID:
      symcheck(expression->fields.isa.what->fields.wht.wht->id,
               INSTANCE_SYMBOL, context);
      break;
    case WHAT_LOCATION:
    case WHAT_ACTOR:
      break;
    default:
      unimpl(&expression->srcp, "Analyzer");
      break;
    }
    break;
  default:
      unimpl(&expression->srcp, "Analyzer");
      break;
  }

  symcheck(expression->fields.isa.class, CLASS_SYMBOL, context);

  expression->type = BOOLEAN_TYPE;
}

/*======================================================================*/
void analyzeExpression(Expression *expression,
                       Context *context)
{
  if (expression == NULL)       /* Ignore empty expressions (syntax error) */
    return;

  switch (expression->kind) {
    
  case WHERE_EXPRESSION:
    analyzeWhereExpression(expression, context);
    break;
    
  case ATTRIBUTE_EXPRESSION:
    analyzeAttributeExpression(expression, context);
    break;

  case BINARY_EXPRESSION:
    analyzeBinaryExpression(expression, context);
    break;
    
  case INTEGER_EXPRESSION:
    expression->type = INTEGER_TYPE;
    break;
    
  case STRING_EXPRESSION:
    expression->type = STRING_TYPE;
    break;
    
  case AGGREGATE_EXPRESSION:
    analyzeAggregate(expression, context);
    break;
    
  case RANDOM_EXPRESSION:
    analyzeRandom(expression, context);
    break;

  case SCORE_EXPRESSION:
    expression->type = INTEGER_TYPE;
    break;

  case WHAT_EXPRESSION:
    analyzeWhatExpression(expression, context);
    break;

  case BETWEEN_EXPRESSION:
    analyzeBetweenExpression(expression, context);
    break;

  case ISA_EXPRESSION:
    analyzeIsaExpression(expression, context);
    break;

  default:
    SYSERR("Unrecognized switch");
    break;
  }
}


/*======================================================================*/
void generateBinaryOperator(Expression *exp)
{
  switch (exp->fields.bin.op) {
  case AND_OPERATOR:
    emit0(I_AND);
    break;
  case OR_OPERATOR:
    emit0(I_OR);
    break;
  case NE_OPERATOR:
    emit0(I_NE);
    break;
  case EQ_OPERATOR:
    if (exp->fields.bin.right->type == STRING_TYPE)
      emit0(I_STREQ);
    else
      emit0(I_EQ);
    break;
  case EXACT_OPERATOR:
    emit0(I_STREXACT);
    break;
  case LE_OPERATOR:
    emit0(I_LE);
    break;
  case GE_OPERATOR:
    emit0(I_GE);
    break;
  case LT_OPERATOR:
    emit0(I_LT);
    break;
  case GT_OPERATOR:
    emit0(I_GT);
    break;
  case PLUS_OPERATOR:
    if (exp->type == INTEGER_TYPE)
      emit0(I_PLUS);
    else if (exp->type == STRING_TYPE)
      emit0(I_CONCAT);
    else
      SYSERR("Unexpected type");
    break;
  case MINUS_OPERATOR:
    emit0(I_MINUS);
    break;
  case MULT_OPERATOR:
    emit0(I_MULT);
    break;
  case DIV_OPERATOR:
    emit0(I_DIV);
    break;
  case CONTAINS_OPERATOR:
    emit0(I_CONTAINS);
    break;
  }
}


/*----------------------------------------------------------------------*/
static void generateBinaryExpression(Expression *exp)
{
  generateExpression(exp->fields.bin.left);
  generateExpression(exp->fields.bin.right);
  generateBinaryOperator(exp);
  if (exp->not) emit0(I_NOT);
}



/*----------------------------------------------------------------------*/
static void generateWhereExpression(Expression *exp)
{
  Where *where = exp->fields.whr.whr;
  Expression *what = exp->fields.whr.wht;

  switch (where->kind) {
  case WHERE_HERE:
    generateExpression(what);
    emit0(I_HERE);
    if (exp->not) emit0(I_NOT);
    return;
  case WHERE_NEAR:
    generateExpression(what);
    emit0(I_NEAR);
    if (exp->not) emit0(I_NOT);
    return;
  case WHERE_IN:
  case WHERE_INSET:
    generateExpression(where->what);
    generateExpression(what);
    if (where->kind == WHERE_IN)
      emit0(I_IN);
    else
      emit0(I_INSET);
    if (exp->not) emit0(I_NOT);
    return;
  case WHERE_AT:
    generateExpression(what);
    emit0(I_WHERE);
    break;
  default:
    unimpl(&exp->srcp, "Code Generator");
    emitConstant(0);
    return;
  }
  
  generateWhere(where);
  emit0(I_EQ);
  if (exp->not) emit0(I_NOT);
}



/*======================================================================*/
void generateAttributeAccess(Expression *exp)
{
  if (exp->type == STRING_TYPE)
    emit0(I_STRATTR);
  else
    emit0(I_ATTRIBUTE);
}

/*======================================================================*/
void generateAttributeReference(Expression *exp) {
  generateId(exp->fields.atr.id);
  generateExpression(exp->fields.atr.wht);
}


/*----------------------------------------------------------------------*/
static void generateAttributeExpression(Expression *exp)
{
  generateAttributeReference(exp);
  generateAttributeAccess(exp);
  if (exp->not) emit0(I_NOT);
}


/*======================================================================*/
void generateRightHandExpression(Expression *exp)
{
  /* Generate a right hand side expression where the left hand 
     has been evaluated so that the stack contains the lhs on top.
     This is used for aggregate and loop filters. */
  switch (exp->kind) {
  case WHERE_EXPRESSION:
    emit0(I_WHERE);
    generateWhere(exp->fields.whr.whr);
    emit0(I_EQ);
    break;
  case ISA_EXPRESSION:
    generateId(exp->fields.isa.class);
    emit0(I_ISA);
    break;
  case BINARY_EXPRESSION:
    generateExpression(exp->fields.bin.right);
    generateBinaryOperator(exp);
    break;
  case ATTRIBUTE_EXPRESSION:
    generateId(exp->fields.atr.id);
    generateAttributeAccess(exp);
    break;
  case BETWEEN_EXPRESSION:
    generateBetweenCheck(exp);
    break;
  default:
    SYSERR("Unimplemented aggregate filter expression");
  }
}

/*----------------------------------------------------------------------*/
static void generateAggregateExpression(Expression *exp)
{
  List *lst;

  switch (exp->fields.agr.kind) {
  case COUNT_AGGREGATE:
  case MAX_AGGREGATE:
  case SUM_AGGREGATE: emitConstant(0); break;
  case MIN_AGGREGATE: emitConstant(-1); break;
  default: SYSERR("Unrecognized switch");
  }
  emit0(I_AGRSTART);

  TRAVERSE(lst,exp->fields.agr.filters) {
    generateRightHandExpression(lst->element.exp);
    emit0(I_AGRCHECK);
  }

  /* Generate attribute code for all aggregates except COUNT */
  if (exp->fields.agr.kind != COUNT_AGGREGATE)
    emitConstant(exp->fields.agr.attribute->code);

  switch (exp->fields.agr.kind) {
  case SUM_AGGREGATE: emit0(I_SUM); break;
  case MAX_AGGREGATE: emit0(I_MAX); break;
  case MIN_AGGREGATE: emit0(I_MIN); break;
  case COUNT_AGGREGATE: emit0(I_COUNT); break;
  default: SYSERR("Unrecognized switch");
  }
  emit0(I_ENDAGR);
}



/*----------------------------------------------------------------------*/
static void generateRandomExpression(Expression *exp)
{
  generateExpression(exp->fields.rnd.to);
  generateExpression(exp->fields.rnd.from);
  emit0(I_RND);
}



/*----------------------------------------------------------------------*/
static void generateScoreExpression(Expression *exp)
{
  emitVariable(V_SCORE);
}


/*----------------------------------------------------------------------*/
static void generateWhatExpression(Expression *exp)
{
  generateWhat(exp->fields.wht.wht);
}



/*======================================================================*/
void generateBetweenCheck(Expression *exp)
{
  generateExpression(exp->fields.btw.low);
  generateExpression(exp->fields.btw.high);
  emit0(I_BTW);
}


/*----------------------------------------------------------------------*/
static void generateBetweenExpression(Expression *exp)
{
  generateExpression(exp->fields.btw.val);
  generateBetweenCheck(exp);
  if (exp->not) emit0(I_NOT);
}


/*----------------------------------------------------------------------*/
static void generateIsaExpression(Expression *exp)
{
  generateExpression(exp->fields.isa.what);
  generateId(exp->fields.isa.class);
  emit0(I_ISA);
  if (exp->not) emit0(I_NOT);
}



/*======================================================================*/
void generateExpression(Expression *exp)
{
  if (exp == NULL) {
    SYSERR("Generating a NULL expression");
    emitConstant(0);
    return;
  }
  
  switch (exp->kind) {
    
  case BINARY_EXPRESSION:
    generateBinaryExpression(exp);
    break;
    
  case WHERE_EXPRESSION:
    generateWhereExpression(exp);
    break;
    
  case ATTRIBUTE_EXPRESSION:
    generateAttributeExpression(exp);
    break;
    
  case INTEGER_EXPRESSION:
    emitConstant(exp->fields.val.val);
    break;
    
  case STRING_EXPRESSION:
    if (!exp->fields.str.encoded)
      encode(&exp->fields.str.fpos, &exp->fields.str.len);
    exp->fields.str.encoded = TRUE;
    emit2(I_GETSTR, exp->fields.str.fpos, exp->fields.str.len);
    break;
    
  case AGGREGATE_EXPRESSION:
    generateAggregateExpression(exp);
    break;
    
  case RANDOM_EXPRESSION:
    generateRandomExpression(exp);
    break;
    
  case SCORE_EXPRESSION:
    generateScoreExpression(exp);
    break;

  case WHAT_EXPRESSION:
    generateWhatExpression(exp);
    break;

  case BETWEEN_EXPRESSION:
    generateBetweenExpression(exp);
    break;

  case ISA_EXPRESSION:
    generateIsaExpression(exp);
    break;
    
  default:
    unimpl(&exp->srcp, "Code Generator");
    emitConstant(0);
    return;
  }
}



/*----------------------------------------------------------------------*/
static void dumpOperator(OperatorKind op)
{
  switch (op) {
  case AND_OPERATOR:
    put("AND");
    break;
  case OR_OPERATOR:
    put("OR");
    break;
  case NE_OPERATOR:
    put("<>");
    break;
  case EQ_OPERATOR:
    put("=");
    break;
  case EXACT_OPERATOR:
    put("==");
    break;
  case LE_OPERATOR:
    put("<=");
    break;
  case GE_OPERATOR:
    put(">=");
    break;
  case LT_OPERATOR:
    put("LT");
    break;
  case GT_OPERATOR:
    put("GT");
    break;
  case PLUS_OPERATOR:
    put("PLUS");
    break;
  case MINUS_OPERATOR:
    put("MINUS");
    break;
  case MULT_OPERATOR:
    put("MULT");
    break;
  case DIV_OPERATOR:
    put("DIV");
    break;
  case CONTAINS_OPERATOR:
    put("CONTAINS");
    break;
  }
}



/*----------------------------------------------------------------------*/
static void dumpAggregateKind(AggregateKind agr)
{
  put(aggregateToString(agr));
}


/*======================================================================*/
char *typeToString(TypeKind typ)
{
  switch (typ) {
  case BOOLEAN_TYPE: return "Boolean"; break;
  case INTEGER_TYPE: return "Integer"; break;
  case STRING_TYPE: return "String"; break;
  case INSTANCE_TYPE: return "Instance"; break;
  case SET_TYPE: return "Set"; break;
  case ERROR_TYPE: return "ERROR"; break;
  case UNINITIALIZED_TYPE: return "UNINITIALIZED"; break;
  }
  return "***ERROR: Unexpected type kind***";
}

/*======================================================================*/
void dumpType(TypeKind typ)
{
  put(typeToString(typ));
}



/*======================================================================*/
void dumpExpression(Expression *exp)
{
  if (exp == NULL) {
    put("NULL");
    return;
  }

  put("EXP: ");
  switch (exp->kind) {
  case BINARY_EXPRESSION:
    put("BIN ");
    break;
  case WHERE_EXPRESSION:
    put("WHR ");
    if (exp->not) put("NOT ");
    break;
  case ATTRIBUTE_EXPRESSION:
    if (exp->not) put("NOT ");
    put("ATR ");
    break;
  case INTEGER_EXPRESSION:
    put("INT ");
    break;
  case STRING_EXPRESSION:
    put("STR ");
    break;
  case AGGREGATE_EXPRESSION:
    put("AGR ");
    break;
  case RANDOM_EXPRESSION:
    put("RND ");
    break;
  case SCORE_EXPRESSION:
    put("SCORE ");
    break;
  case WHAT_EXPRESSION:
    put("WHT ");
    break; 
  case BETWEEN_EXPRESSION:
    if (exp->not) put("NOT ");
    put("BTW ");
    break;
  case ISA_EXPRESSION:
    if (exp->not) put("NOT ");
    put("ISA ");
    break;
  default:
    put("*** Expression kind not implemented in dump() *** ");
    break;
  }

  dumpSrcp(&exp->srcp);
  indent();
  put("type: "); dumpType(exp->type); nl();
  put("class: "); dumpPointer(exp->class); nl();

  switch (exp->kind) {
  case WHERE_EXPRESSION:
    put("wht: "); dumpExpression(exp->fields.whr.wht); nl();
    put("whr: "); dumpWhere(exp->fields.whr.whr);
    break;
  case ATTRIBUTE_EXPRESSION:
    put("wht: "); dumpExpression(exp->fields.atr.wht); nl();
    put("atr: "); dumpId(exp->fields.atr.id);
    break;
  case INTEGER_EXPRESSION:
    put("val: "); dumpInt(exp->fields.val.val);
    break;
  case STRING_EXPRESSION:
    put("fpos: "); dumpInt(exp->fields.str.fpos); nl();
    put("len: "); dumpInt(exp->fields.str.len);
    break;
  case BINARY_EXPRESSION:
    put("operator: "); dumpOperator(exp->fields.bin.op); nl();
    put("left: "); dumpExpression(exp->fields.bin.left); nl();
    put("right: "); dumpExpression(exp->fields.bin.right);
    break;
  case AGGREGATE_EXPRESSION:
    put("kind: "); dumpAggregateKind(exp->fields.agr.kind); nl();
    put("attribute: "); dumpId(exp->fields.agr.attribute); nl();
    put("filters: "); dumpList(exp->fields.agr.filters, EXPRESSION_LIST);
    break;
  case RANDOM_EXPRESSION:
    put("from: "); dumpExpression(exp->fields.rnd.from); nl();
    put("to: "); dumpExpression(exp->fields.rnd.to);
    break;
  case WHAT_EXPRESSION:
    put("wht: "); dumpWhat(exp->fields.wht.wht);
    break;
  case SCORE_EXPRESSION:
    break;
  case BETWEEN_EXPRESSION:
    put("val: "); dumpExpression(exp->fields.btw.val); nl();
    put("low: "); dumpExpression(exp->fields.btw.low); nl();
    put("high: "); dumpExpression(exp->fields.btw.high);
    break;
  case ISA_EXPRESSION:
    put("wht: "); dumpExpression(exp->fields.isa.what); nl();
    put("id: "); dumpId(exp->fields.isa.class);
    break;
  }
  out();
}
