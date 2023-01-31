#pragma once

#include "base/exceptions.h"

class ExprVar;
class ExprVarProperty;
class ExprConstant;
class ExprAddition;
class ExprDivision;
class ExprModulo;
class ExprMultiplication;
class ExprSubtraction;
class ExprUnaryMinus;
class ExprUnaryPlus;
class ExprEquals;
class ExprGreaterOrEquals;
class ExprGreater;
class ExprIs;
class ExprLessOrEquals;
class ExprLess;
class ExprNotEquals;
class ExprAnd;
class ExprNot;
class ExprOr;

namespace SPARQL {

class ExprVar;
class ExprTerm;

class ExprNot;
class ExprUnaryMinus;
class ExprUnaryPlus;
class ExprMultiplication;
class ExprDivision;
class ExprAddition;
class ExprSubtraction;
class ExprAnd;
class ExprOr;
class ExprEqual;
class ExprNotEqual;
class ExprLess;
class ExprGreater;
class ExprLessOrEqual;
class ExprGreaterOrEqual;

class ExprIn;
class ExprNotIn;

class ExprAggAvg;
class ExprAggCountAll;
class ExprAggCount;
class ExprAggGroupConcat;
class ExprAggMax;
class ExprAggMin;
class ExprAggSample;
class ExprAggSum;

class ExprAbs;
class ExprBNode;
class ExprBound;
class ExprCeil;
class ExprCoalesce;
class ExprConcat;
class ExprContains;
class ExprDatatype;
class ExprDay;
class ExprEncodeForUri;
class ExprExists;
class ExprFloor;
class ExprHours;
class ExprIf;
class ExprIRI;
class ExprIsBlank;
class ExprIsIRI;
class ExprIsLiteral;
class ExprIsNumeric;
class ExprIsURI;
class ExprLang;
class ExprLangMatches;
class ExprLCase;
class ExprMD5;
class ExprMinutes;
class ExprMonth;
class ExprNotExists;
class ExprNow;
class ExprRand;
class ExprRegex;
class ExprReplace;
class ExprRound;
class ExprSameTerm;
class ExprSeconds;
class ExprSHA1;
class ExprSHA256;
class ExprSHA384;
class ExprSHA512;
class ExprStrAfter;
class ExprStrBefore;
class ExprStrDT;
class ExprStrEnds;
class ExprStrLang;
class ExprStrLen;
class ExprStrStarts;
class ExprStrUUID;
class ExprStr;
class ExprSubStr;
class ExprTimezone;
class ExprTZ;
class ExprUCase;
class ExprURI;
class ExprUUID;
class ExprYear;
} // namespace SPARQL

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;

    virtual void visit(ExprVar&)             { throw LogicException("visit ExprVar not implemented"); }
    virtual void visit(ExprVarProperty&)     { throw LogicException("visit ExprVarProperty not implemented"); }
    virtual void visit(ExprConstant&)        { throw LogicException("visit ExprConstant not implemented"); }
    virtual void visit(ExprAddition&)        { throw LogicException("visit ExprAddition not implemented"); }
    virtual void visit(ExprDivision&)        { throw LogicException("visit ExprDivision not implemented"); }
    virtual void visit(ExprModulo&)          { throw LogicException("visit ExprModulo not implemented"); }
    virtual void visit(ExprMultiplication&)  { throw LogicException("visit ExprMultiplication not implemented"); }
    virtual void visit(ExprSubtraction&)     { throw LogicException("visit ExprSubtraction not implemented"); }
    virtual void visit(ExprUnaryMinus&)      { throw LogicException("visit ExprUnaryMinus not implemented"); }
    virtual void visit(ExprUnaryPlus&)       { throw LogicException("visit ExprUnaryPlus not implemented"); }
    virtual void visit(ExprEquals&)          { throw LogicException("visit ExprEquals not implemented"); }
    virtual void visit(ExprGreaterOrEquals&) { throw LogicException("visit ExprGreaterOrEquals not implemented"); }
    virtual void visit(ExprGreater&)         { throw LogicException("visit ExprGreater not implemented"); }
    virtual void visit(ExprIs&)              { throw LogicException("visit ExprIs not implemented"); }
    virtual void visit(ExprLessOrEquals&)    { throw LogicException("visit ExprLessOrEquals not implemented"); }
    virtual void visit(ExprLess&)            { throw LogicException("visit ExprLess not implemented"); }
    virtual void visit(ExprNotEquals&)       { throw LogicException("visit ExprNotEquals not implemented"); }
    virtual void visit(ExprAnd&)             { throw LogicException("visit ExprAnd not implemented"); }
    virtual void visit(ExprNot&)             { throw LogicException("visit ExprNot not implemented"); }
    virtual void visit(ExprOr&)              { throw LogicException("visit ExprOr not implemented"); }

    // SPARQL exprs
    virtual void visit(SPARQL::ExprVar&)            { throw LogicException("visit SPARQL::ExprVar not implemented"); }
    virtual void visit(SPARQL::ExprNot&)            { throw LogicException("visit SPARQL::ExprNot not implemented"); }
    virtual void visit(SPARQL::ExprUnaryMinus&)     { throw LogicException("visit SPARQL::ExprUnaryMinus not implemented"); }
    virtual void visit(SPARQL::ExprUnaryPlus&)      { throw LogicException("visit SPARQL::ExprUnaryPlus not implemented"); }
    virtual void visit(SPARQL::ExprMultiplication&) { throw LogicException("visit SPARQL::ExprMultiplication not implemented"); }
    virtual void visit(SPARQL::ExprDivision&)       { throw LogicException("visit SPARQL::ExprDivision not implemented"); }
    virtual void visit(SPARQL::ExprAddition&)       { throw LogicException("visit SPARQL::ExprAddition not implemented"); }
    virtual void visit(SPARQL::ExprSubtraction&)    { throw LogicException("visit SPARQL::ExprSubtraction not implemented"); }
    virtual void visit(SPARQL::ExprAnd&)            { throw LogicException("visit SPARQL::ExprAnd not implemented"); }
    virtual void visit(SPARQL::ExprOr&)             { throw LogicException("visit SPARQL::ExprOr not implemented"); }
    virtual void visit(SPARQL::ExprLess&)           { throw LogicException("visit SPARQL::ExprLess not implemented"); }
    virtual void visit(SPARQL::ExprGreater&)        { throw LogicException("visit SPARQL::ExprGreater not implemented"); }
    virtual void visit(SPARQL::ExprEqual&)          { throw LogicException("visit SPARQL::ExprEqual not implemented"); }
    virtual void visit(SPARQL::ExprNotEqual&)       { throw LogicException("visit SPARQL::ExprNotEqual not implemented"); }
    virtual void visit(SPARQL::ExprLessOrEqual&)    { throw LogicException("visit SPARQL::ExprLessOrEqual not implemented"); }
    virtual void visit(SPARQL::ExprGreaterOrEqual&) { throw LogicException("visit SPARQL::ExprGreaterOrEqual not implemented"); }

    virtual void visit(SPARQL::ExprTerm&)           { throw LogicException("visit SPARQL::ExprTerm not implemented"); }
    virtual void visit(SPARQL::ExprIn&)             { throw LogicException("visit SPARQL::ExprIn not implemented"); }
    virtual void visit(SPARQL::ExprNotIn&)          { throw LogicException("visit SPARQL::ExprNotIn not implemented"); }
    virtual void visit(SPARQL::ExprAggAvg&)         { throw LogicException("visit SPARQL::ExprAggAvg not implemented"); }
    virtual void visit(SPARQL::ExprAggCount&)       { throw LogicException("visit SPARQL::ExprAggCount not implemented"); }
    virtual void visit(SPARQL::ExprAggCountAll&)    { throw LogicException("visit SPARQL::ExprAggCountAll not implemented"); }
    virtual void visit(SPARQL::ExprAggGroupConcat&) { throw LogicException("visit SPARQL::ExprAggGroupConcat not implemented"); }
    virtual void visit(SPARQL::ExprAggMax&)         { throw LogicException("visit SPARQL::ExprAggMax not implemented"); }
    virtual void visit(SPARQL::ExprAggMin&)         { throw LogicException("visit SPARQL::ExprAggMin not implemented"); }
    virtual void visit(SPARQL::ExprAggSample&)      { throw LogicException("visit SPARQL::ExprAggSample not implemented"); }
    virtual void visit(SPARQL::ExprAggSum&)         { throw LogicException("visit SPARQL::ExprAggSum not implemented"); }
    virtual void visit(SPARQL::ExprAbs&)            { throw LogicException("visit SPARQL::ExprAbs not implemented"); }
    virtual void visit(SPARQL::ExprBNode&)          { throw LogicException("visit SPARQL::ExprBNode not implemented"); }
    virtual void visit(SPARQL::ExprBound&)          { throw LogicException("visit SPARQL::ExprBound not implemented"); }
    virtual void visit(SPARQL::ExprCeil&)           { throw LogicException("visit SPARQL::ExprCeil not implemented"); }
    virtual void visit(SPARQL::ExprCoalesce&)       { throw LogicException("visit SPARQL::ExprCoalesce not implemented"); }
    virtual void visit(SPARQL::ExprConcat&)         { throw LogicException("visit SPARQL::ExprConcat not implemented"); }
    virtual void visit(SPARQL::ExprContains&)       { throw LogicException("visit SPARQL::ExprContains not implemented"); }
    virtual void visit(SPARQL::ExprDatatype&)       { throw LogicException("visit SPARQL::ExprDatatype not implemented"); }
    virtual void visit(SPARQL::ExprDay&)            { throw LogicException("visit SPARQL::ExprDay not implemented"); }
    virtual void visit(SPARQL::ExprEncodeForUri&)   { throw LogicException("visit SPARQL::ExprEncodeForUri not implemented"); }
    virtual void visit(SPARQL::ExprExists&)         { throw LogicException("visit SPARQL::ExprExists not implemented"); }
    virtual void visit(SPARQL::ExprFloor&)          { throw LogicException("visit SPARQL::ExprFloor not implemented"); }
    virtual void visit(SPARQL::ExprHours&)          { throw LogicException("visit SPARQL::ExprHours not implemented"); }
    virtual void visit(SPARQL::ExprIf&)             { throw LogicException("visit SPARQL::ExprIf not implemented"); }
    virtual void visit(SPARQL::ExprIRI&)            { throw LogicException("visit SPARQL::ExprIRI not implemented"); }
    virtual void visit(SPARQL::ExprIsBlank&)        { throw LogicException("visit SPARQL::ExprIsBlank not implemented"); }
    virtual void visit(SPARQL::ExprIsIRI&)          { throw LogicException("visit SPARQL::ExprIsIRI not implemented"); }
    virtual void visit(SPARQL::ExprIsLiteral&)      { throw LogicException("visit SPARQL::ExprIsLiteral not implemented"); }
    virtual void visit(SPARQL::ExprIsNumeric&)      { throw LogicException("visit SPARQL::ExprIsNumeric not implemented"); }
    virtual void visit(SPARQL::ExprIsURI&)          { throw LogicException("visit SPARQL::ExprIsURI not implemented"); }
    virtual void visit(SPARQL::ExprLang&)           { throw LogicException("visit SPARQL::ExprLang not implemented"); }
    virtual void visit(SPARQL::ExprLangMatches&)    { throw LogicException("visit SPARQL::ExprLangMatches not implemented"); }
    virtual void visit(SPARQL::ExprLCase&)          { throw LogicException("visit SPARQL::ExprLCase not implemented"); }
    virtual void visit(SPARQL::ExprMD5&)            { throw LogicException("visit SPARQL::ExprMD5 not implemented"); }
    virtual void visit(SPARQL::ExprMinutes&)        { throw LogicException("visit SPARQL::ExprMinutes not implemented"); }
    virtual void visit(SPARQL::ExprMonth&)          { throw LogicException("visit SPARQL::ExprMonth not implemented"); }
    virtual void visit(SPARQL::ExprNotExists&)      { throw LogicException("visit SPARQL::ExprNotExists not implemented"); }
    virtual void visit(SPARQL::ExprNow&)            { throw LogicException("visit SPARQL::ExprNow not implemented"); }
    virtual void visit(SPARQL::ExprRand&)           { throw LogicException("visit SPARQL::ExprRand not implemented"); }
    virtual void visit(SPARQL::ExprRegex&)          { throw LogicException("visit SPARQL::ExprRegex not implemented"); }
    virtual void visit(SPARQL::ExprReplace&)        { throw LogicException("visit SPARQL::ExprReplace not implemented"); }
    virtual void visit(SPARQL::ExprRound&)          { throw LogicException("visit SPARQL::ExprRound not implemented"); }
    virtual void visit(SPARQL::ExprSameTerm&)       { throw LogicException("visit SPARQL::ExprSameTerm not implemented"); }
    virtual void visit(SPARQL::ExprSeconds&)        { throw LogicException("visit SPARQL::ExprSeconds not implemented"); }
    virtual void visit(SPARQL::ExprSHA1&)           { throw LogicException("visit SPARQL::ExprSHA1 not implemented"); }
    virtual void visit(SPARQL::ExprSHA256&)         { throw LogicException("visit SPARQL::ExprSHA256 not implemented"); }
    virtual void visit(SPARQL::ExprSHA384&)         { throw LogicException("visit SPARQL::ExprSHA384 not implemented"); }
    virtual void visit(SPARQL::ExprSHA512&)         { throw LogicException("visit SPARQL::ExprSHA512 not implemented"); }
    virtual void visit(SPARQL::ExprStrAfter&)       { throw LogicException("visit SPARQL::ExprStrAfter not implemented"); }
    virtual void visit(SPARQL::ExprStrBefore&)      { throw LogicException("visit SPARQL::ExprStrBefore not implemented"); }
    virtual void visit(SPARQL::ExprStrDT&)          { throw LogicException("visit SPARQL::ExprStrDT not implemented"); }
    virtual void visit(SPARQL::ExprStrEnds&)        { throw LogicException("visit SPARQL::ExprStrEnds not implemented"); }
    virtual void visit(SPARQL::ExprStrLang&)        { throw LogicException("visit SPARQL::ExprStrLang not implemented"); }
    virtual void visit(SPARQL::ExprStrLen&)         { throw LogicException("visit SPARQL::ExprStrLen not implemented"); }
    virtual void visit(SPARQL::ExprStrStarts&)      { throw LogicException("visit SPARQL::ExprStrStarts not implemented"); }
    virtual void visit(SPARQL::ExprStrUUID&)        { throw LogicException("visit SPARQL::ExprStrUUID not implemented"); }
    virtual void visit(SPARQL::ExprStr&)            { throw LogicException("visit SPARQL::ExprStr not implemented"); }
    virtual void visit(SPARQL::ExprSubStr&)         { throw LogicException("visit SPARQL::ExprSubStr not implemented"); }
    virtual void visit(SPARQL::ExprTimezone&)       { throw LogicException("visit SPARQL::ExprTimezone not implemented"); }
    virtual void visit(SPARQL::ExprTZ&)             { throw LogicException("visit SPARQL::ExprTZ not implemented"); }
    virtual void visit(SPARQL::ExprUCase&)          { throw LogicException("visit SPARQL::ExprUCase not implemented"); }
    virtual void visit(SPARQL::ExprURI&)            { throw LogicException("visit SPARQL::ExprURI not implemented"); }
    virtual void visit(SPARQL::ExprUUID&)           { throw LogicException("visit SPARQL::ExprUUID not implemented"); }
    virtual void visit(SPARQL::ExprYear&)           { throw LogicException("visit SPARQL::ExprYear not implemented"); }
};
