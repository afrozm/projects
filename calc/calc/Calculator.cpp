//
//  Calculator.cpp
//  calc
//
//  Created by Afroz Muzammil on 23/11/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//
#include "stdafx.h"
#include "Calculator.h"
#include <cmath>


#define PI 3.14159265358979323846
#define E 2.71828182845904523536

Number::Number(const char * number, int *outIndex /*= NULL*/)
	: mType(Invalid)
{
	SetNumber(number, outIndex);
}

Number::Number(long long no/*=0*/)
	: mType(Long)
{
	SetNumber(no);
}

Number::Number(double number)
	: mType(Double)
{
	SetNumber(number);
}

std::string Number::GetAsString(StringType type /* = Normal */, int decimalPrecision/* =2 */) const
{
	std::string outStr;

	switch (mType)
	{
	case Number::Invalid:
		outStr = "NaN";
		break;
	case Infinity:
		outStr = "∞";
		break;
	case Number::Long:
		switch (type)
		{
		case Number::Normal:
			outStr = std::to_string(GetLong());
			break;
		case Number::Binary:
			break;
		case Number::Octal:
			break;
		case Number::Hex:
		{
			char str[128] = { 0 };
			sprintf_s(str, sizeof(str) / sizeof(str[0]), "0x%llx", GetLong());
			outStr = str;
		}
			break;
		default:
			break;
		}
		break;
	case Number::Double:
	{
		char str[128] = { 0 };
		if (decimalPrecision > 0) {
			sprintf_s(str, sizeof(str) / sizeof(str[0]), "%%.%df", decimalPrecision);
			outStr = str;
			sprintf_s(str, sizeof(str) / sizeof(str[0]), outStr.c_str(), GetDouble());
			outStr = str;
		}
		else
			outStr = std::to_string(GetDouble());
	}
		
		break;
	default:
		break;
	}

	return outStr;
}

void Number::SetNumber(const char *number, int *outIndex /*= NULL*/)
{
	Reset();
	if (number == NULL || *number == 0)
		return;
	bool bMinus(*number == '-');
	if (bMinus)
		++number;
	size_t idx = 0;
	try {
		long long np(std::stoll(number, &idx, 0));
		if (bMinus)
			np = -np;
		SetNumber(np);
	}
	catch (...) {}
	if (number[idx] == '.') {
		try {
			double nu = std::stod(number, &idx);
			if (bMinus)
				nu = -nu;
			SetNumber(nu);
		} catch (...) {}
	}
	if (bMinus)
		++idx;
	if (GetType() == Invalid
		&& *number == '∞')
		Reset(Infinity);
	if (outIndex)
		*outIndex = (int)idx;
}

void Number::SetNumber(double number)
{
	Reset(Double);
	mNumber.mdNumber = number;
	// if it is xxx.0 then set to xxx
	// like for 2.0 it set to 2
	if (GetDouble() == (double)GetLong())
		SetNumber(GetLong());
}

void Number::SetNumber(long long number)
{
	Reset(Long);
	mNumber.mllNumber = number;
}

double Number::GetDouble() const
{
	if (GetType() == Long)
		return (double)mNumber.mllNumber;
	return mNumber.mdNumber;
}

long long Number::GetLong() const
{
	if (GetType() == Double)
		return (long long)mNumber.mdNumber;
	return mNumber.mllNumber;
}

Number::operator double() const
{
	return GetDouble();
}

Number::operator long long() const
{
	return GetLong();
}

void Number::Reset(NumberType type /* = Invalid */)
{
	mType = type;
	mNumber.mllNumber = 0;
	mNumber.mdNumber = 0;
}

Number Number::operator-() const
{
	Number out(*this);
	if (out.GetType() == Long)
		out.SetNumber(-out.GetLong());
	else
		out.SetNumber(-out.GetDouble());
	return out;
}

bool Number::operator!() const
{
	if (IsNaN())
		return true;
	if (GetType() == Infinity)
		return false;
	if (GetType() == Double)
		return GetDouble() == 0.0;
	return !GetLong();
}

Number operator / (const Number & n1, const Number & n2)
{
	if (n1.GetType() == Number::Infinity)
		return n1;
	if (n2.GetType() == Number::Infinity)
		return 0LL;
	if (!n2) {
		Number ret;
		ret.Reset(Number::Infinity);
		return ret;
	}
	return n1.GetDouble() / n2.GetDouble(); 
}


#define DEFINE_NUMBER_OPERATOR(op) \
Number operator op (const Number & n1, const Number & n2) \
{\
	if (n1.GetType() == n2.GetType()\
		&& n1.GetType() == Number::Long)\
		return n1.GetLong() op n2.GetLong();\
	return n1.GetDouble() op n2.GetDouble();\
}

DEFINE_NUMBER_OPERATOR(+)
DEFINE_NUMBER_OPERATOR(-)
DEFINE_NUMBER_OPERATOR(*)

#define DEFINE_BOOL_OPERATOR(op) \
bool operator op (const Number & n1, const Number & n2) \
{\
	if (n1.GetType() == n2.GetType()\
		&& n1.GetType() == Number::Long)\
		return n1.GetLong() op n2.GetLong();\
	return n1.GetDouble() op n2.GetDouble();\
}\

DEFINE_BOOL_OPERATOR(==)
DEFINE_BOOL_OPERATOR(!=)
DEFINE_BOOL_OPERATOR(<)
DEFINE_BOOL_OPERATOR(>)
DEFINE_BOOL_OPERATOR(<=)
DEFINE_BOOL_OPERATOR(>=)


#define DEFINE_LOGICAL_OPERATOR(op) \
Number operator op (const Number & n1, const Number & n2) \
{\
	return n1.GetLong() op n2.GetLong();\
}

DEFINE_LOGICAL_OPERATOR(%)
DEFINE_LOGICAL_OPERATOR(&)
DEFINE_LOGICAL_OPERATOR(|)
DEFINE_LOGICAL_OPERATOR(^)
DEFINE_LOGICAL_OPERATOR(<<)
DEFINE_LOGICAL_OPERATOR(>>)


//////////////////////////////////////////// Operator ////////////////////////////////////////////

class LeftParenthesisOperator : public Operator
{
public:
	LeftParenthesisOperator() : Operator(0, 0, "(") {}
	virtual Number Operate(const VecNumbers &numbers) const { return 0LL; }
};

class CommaOperator : public Operator
{
public:
	CommaOperator() : Operator(2, 16, ",") {}
	virtual Number Operate(const VecNumbers &numbers) const { return 0LL; }
};


#define DEFINE_NUMBER_OPERATOR_CLASS(cn,op,p)\
class cn##Operator : public Operator\
{\
public:\
	cn##Operator() : Operator(2, p, #op) {}\
	virtual Number Operate(const VecNumbers &numbers) const { return numbers[0] op numbers[1]; }\
};

DEFINE_NUMBER_OPERATOR_CLASS(Plus, +, 6)
DEFINE_NUMBER_OPERATOR_CLASS(Minus, -, 6)
DEFINE_NUMBER_OPERATOR_CLASS(Multiply, *, 5)
DEFINE_NUMBER_OPERATOR_CLASS(Devide, /, 5)
DEFINE_NUMBER_OPERATOR_CLASS(Remainder, %, 5)
DEFINE_NUMBER_OPERATOR_CLASS(And, &, 10)
DEFINE_NUMBER_OPERATOR_CLASS(Or, |, 12)
DEFINE_NUMBER_OPERATOR_CLASS(Xor, ^, 11)

#define DEFINE_BOOL_OPERATOR_CLASS(cn,op,p)\
class cn##Operator : public Operator\
{\
public:\
	cn##Operator() : Operator(2, p, #op) {}\
	virtual Number Operate(const VecNumbers &numbers) const { return numbers[0] op numbers[1] ? 1LL : 0LL; }\
};



DEFINE_BOOL_OPERATOR_CLASS(Equal, ==, 9)
DEFINE_BOOL_OPERATOR_CLASS(NotEqual, !=, 9)
DEFINE_BOOL_OPERATOR_CLASS(LesserThan, <, 8)
DEFINE_BOOL_OPERATOR_CLASS(GreaterThan, >, 8)
DEFINE_BOOL_OPERATOR_CLASS(LessrThanOrEqual, <=, 8)
DEFINE_BOOL_OPERATOR_CLASS(GreaterThanEQual, >=, 8)

#define DEFINE_UNARY_OPERATOR_CLASS(cn,op,p)\
class cn##Operator : public Operator\
{\
public:\
	cn##Operator() : Operator(1, p, #op) {}\
	virtual Number Operate(const VecNumbers &numbers) const { return  op numbers[0]; }\
};

DEFINE_UNARY_OPERATOR_CLASS(UnaryMinus, -, 3)
DEFINE_UNARY_OPERATOR_CLASS(Tilde, ~, 3)

class NotOperator : public Operator
{
public:
	NotOperator() : Operator(1, 3, "!") {}
	virtual Number Operate(const VecNumbers &numbers) const { return  !numbers[0] ? 1LL : 0LL; }
};

typedef Number(*FuntionOperator)(const VecNumbers &numbers);

#define DEFINE_FUNCTION_OPERATOR_CLASS(fn,no)\
class fn##Operator : public Operator\
{\
public:\
	fn##Operator() : Operator(no, 2, #fn) {}\
	virtual Number Operate(const VecNumbers &numbers) const { return fn(numbers); }\
};

static Number fact(const VecNumbers &numbers)
{
	Number out;

	if (numbers.size() == 1) {
		long long f = numbers[0].GetLong();
		long long c(f);
		while (--c > 1)
			f *= c;
		out = f;
	}

	return out;
}
DEFINE_FUNCTION_OPERATOR_CLASS(fact,1)

static Number avg(const VecNumbers &numbers)
{
	Number out(0LL);

	if (numbers.size() > 0) {
		for (auto &no : numbers)
			out = out + no;

		out = out / Number((long long)numbers.size());
	}

	return out;
}
DEFINE_FUNCTION_OPERATOR_CLASS(avg, -1)


#define DEFINE_CONSTANT_OPERATOR_FN(n,c)\
static Number n(const VecNumbers & /*numbers*/)\
{\
	return c;\
}
#define DEFINE_CONSTANT_OPERATOR_CLASS(n,c)\
DEFINE_CONSTANT_OPERATOR_FN(n,c)\
DEFINE_FUNCTION_OPERATOR_CLASS(n,0)

DEFINE_CONSTANT_OPERATOR_CLASS(pi, PI)
DEFINE_CONSTANT_OPERATOR_CLASS(e, E)

class OperatorManager
{
public:
	static OperatorManager& GetInstance();
	const Operator* GetOperator(const char *str);
	const Operator* GetNoOperator() const { return &mNoOp; }
	~OperatorManager();
private:
	OperatorManager() {}
	void Init();
	std::vector<Operator*> mOperators;
	Operator mNoOp;
};
OperatorManager::~OperatorManager()
{
	for (auto op : mOperators)
		delete op;
}
OperatorManager& OperatorManager::GetInstance()
{
	static OperatorManager sOperatorManager;
	return sOperatorManager;
}


#define OPM_ADD_OPERATOR(op) mOperators.push_back(new op##Operator)
void OperatorManager::Init()
{
	if (mOperators.empty()) {
		// Arithmetic operators
		OPM_ADD_OPERATOR(Plus);
		OPM_ADD_OPERATOR(Minus);
		OPM_ADD_OPERATOR(Multiply);
		OPM_ADD_OPERATOR(Devide);
		OPM_ADD_OPERATOR(Remainder);
		OPM_ADD_OPERATOR(And);
		OPM_ADD_OPERATOR(Or);
		OPM_ADD_OPERATOR(Xor);
		OPM_ADD_OPERATOR(Equal);
		OPM_ADD_OPERATOR(NotEqual);
		OPM_ADD_OPERATOR(LesserThan);
		OPM_ADD_OPERATOR(GreaterThan);
		OPM_ADD_OPERATOR(LessrThanOrEqual);
		OPM_ADD_OPERATOR(GreaterThanEQual);
		OPM_ADD_OPERATOR(Comma);
		// Special check for UnaryMinus
		OPM_ADD_OPERATOR(Tilde);

		// Functions
		OPM_ADD_OPERATOR(fact);
		OPM_ADD_OPERATOR(pi);
		OPM_ADD_OPERATOR(e);
		OPM_ADD_OPERATOR(avg);
		// Sort as per longest names
		std::sort(mOperators.begin(), mOperators.end(), [](Operator *op1, Operator *op2) -> bool
		{
			return strlen(op1->Name()) > strlen(op2->Name());
		});
	}
}
const Operator* OperatorManager::GetOperator(const char *str)
{
	Init();
	std::string opStr(str);
	for (auto op : mOperators)
		if (opStr.find(op->Name()) == 0)
			return op;
	return NULL;
}



//////////////////////////////////////////// Calculator ////////////////////////////////////////////
Calculator::~Calculator()
{

}



Number Calculator::EvaluateExpression(const char * expression)
{
	Number outNumber;

	if (expression == NULL || *expression == 0)
		return outNumber;
	std::string expr;
	{
		std::string str(expression);
		const char *whiteSpaces[] = { " ", "\t", "\r", "\n" };
		for (auto whiteSpace : whiteSpaces)
			StringUtils::Replace(str, std::string(whiteSpace), std::string(""));
		expr = StringUtils::ToLower(str);
	}
	//printf("%s\n", expr.c_str());
	VecExpressionEntity postFix;
	InfixToPostFix(expr.c_str(), postFix);
	// Print post fix
	//{
	//	for (const auto &ee : postFix)
	//		printf("%s ", ee.GetAsString().c_str());
	//	printf("\n");
	//}
	outNumber = EvalPostFixExpression(postFix);

	return outNumber;
}

bool Calculator::InfixToPostFix(const std::string & inExpStr, VecExpressionEntity & outPostFixExpression)
{
	bool bSuccess(true);

	const char *pExpr(inExpStr.c_str());
	std::vector<const Operator*> opStack;
	OperatorManager &opManager(OperatorManager::GetInstance());
	LeftParenthesisOperator leftParenthesisOperator;
	const Operator *oplp(&leftParenthesisOperator);

	while (*pExpr) {
		size_t incrL(1);
		const Operator *op = opManager.GetOperator(pExpr);
		if (op) {
			incrL = strlen(op->Name());
			if (op->GetNumberOfOperands() == 0) // Constants - push
				outPostFixExpression.push_back(op->Operate(VecNumbers()));
			else {
				while (!opStack.empty() && opStack.back() != oplp
					&& opStack.back()->Precendence() <= op->Precendence()) {
					outPostFixExpression.push_back(opStack.back());
					opStack.pop_back();
				}
				if (op->Name() != std::string(","))
					opStack.push_back(op);
			}
		}
		else if (*pExpr == '(') {
			opStack.push_back(oplp);
			outPostFixExpression.push_back(opManager.GetNoOperator());
		}
		else if (*pExpr == ')') {
			while (!opStack.empty() && opStack.back() != oplp)
			{
				outPostFixExpression.push_back(opStack.back());
				opStack.pop_back();
			}
			if (!opStack.empty())
				opStack.pop_back();
			else
				mErrorStr += "Error: Parenthesis mismatch at " + std::string(pExpr, 8) + "\r\n";
			if (!opStack.empty() && opStack.back()->GetNumberOfOperands() >= 0) {
				size_t index(0);
				for (auto rit = outPostFixExpression.rbegin(); rit != outPostFixExpression.rend(); ++rit) {
					++index;
					if (rit->IsOperator() && rit->GetAsString().empty()) {
						outPostFixExpression.erase(outPostFixExpression.begin() + (outPostFixExpression.size()-index));
						break;
					}
				}
			}
		}
		else {
			int idx(0);
			Number number(pExpr, &idx);
			if (idx > 0) {
				outPostFixExpression.push_back(number);
				incrL = idx;
			}
			else
				mErrorStr += "Error: Number is expected at " + std::string(pExpr, 8) + "\r\n";

		}
		pExpr += incrL;
	}
	while (!opStack.empty())
	{
		if (opStack.back() == oplp) {// un expected (
			mErrorStr += "Error: Unexpected left parenthesis at " + std::string(inExpStr.c_str() + inExpStr.rfind('('), 8) + "\r\n";
		}
		else
			outPostFixExpression.push_back(opStack.back());
		opStack.pop_back();
	}

	return bSuccess;
}

Number Calculator::EvalPostFixExpression(const VecExpressionEntity & inPostFixExpression)
{
	// 5  1 5 + 2 3 * 8 6 - - 7 avg / 4 *
	VecExpressionEntity stack;
	for (const auto &ee : inPostFixExpression) {
		// not empty operator
		if (ee.IsOperator() && !ee.GetAsString().empty()) {
			int nop(ee.m_pOperator->GetNumberOfOperands());
			VecNumbers operands;
			if (nop != 0) {
				// Push all numbers
				while (!stack.empty() && !stack.back().IsOperator())
				{
					operands.insert(operands.begin(), stack.back().mNumber);
					stack.pop_back();
					if (nop > 0) {
						--nop;
						if (nop <= 0)
							break;
					}
				}
				if (nop < 0 && !stack.empty() && stack.back().IsOperator())
					stack.pop_back();
			}
			stack.push_back(ee.m_pOperator->Operate(operands));
		}
		else
			stack.push_back(ee);
	}
	while (!stack.empty() && stack.back().IsOperator())
		stack.pop_back();
	Number outNumber;
	if (!stack.empty())
		outNumber = stack.back().mNumber;
	else
		outNumber.Reset(Number::Invalid);

	return outNumber;
}


