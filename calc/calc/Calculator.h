//
//  Calculator.hpp
//  calc
//
//  Created by Afroz Muzammil on 23/11/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#ifndef Calculator_hpp
#define Calculator_hpp

#include <string>

class Number {
public:
    Number(const char *number, int *outIndex = NULL);
    Number(long long no=0);
    Number(double number);
    enum StringType {
        Normal,
        Binary,
        Octal,
        Hex,
    };
    std::string GetAsString(StringType type = Normal, int decimalPrecision=2) const;
	void SetNumber(const char *number, int *outIndex = NULL);
	void SetNumber(double number);
	void SetNumber(long long number);
	double GetDouble() const;
    long long GetLong() const;
	operator double() const;
	operator long long() const;
    enum NumberType {
        Invalid, // NaN
		Infinity,
        Long,
        Double
    } ;
	NumberType GetType() const { return mType; }
	bool IsNaN() const { return GetType() == Invalid; }
	void Reset(NumberType type = Invalid);
	bool operator !()const;
	operator bool() const { return !!*this; }
	Number operator~() const { return ~GetLong(); }
	Number operator-() const;
private:
    union {
        long long mllNumber;
        double mdNumber;
    } mNumber;
    NumberType mType;
};
#define DECLARE_NUMBER_OPERATOR(op) \
Number operator op (const Number & n1, const Number & n2);

DECLARE_NUMBER_OPERATOR(+)
DECLARE_NUMBER_OPERATOR(-)
DECLARE_NUMBER_OPERATOR(*)
DECLARE_NUMBER_OPERATOR(/)
DECLARE_NUMBER_OPERATOR(%)

// Logical operators
DECLARE_NUMBER_OPERATOR(%)
DECLARE_NUMBER_OPERATOR(&)
DECLARE_NUMBER_OPERATOR(|)
DECLARE_NUMBER_OPERATOR(^)
DECLARE_NUMBER_OPERATOR(<<)
DECLARE_NUMBER_OPERATOR(>>)


#define DECLARE_BOOL_OPERATOR(op) \
bool operator op (const Number & n1, const Number & n2);

DECLARE_BOOL_OPERATOR(==)
DECLARE_BOOL_OPERATOR(!=)
DECLARE_BOOL_OPERATOR(<)
DECLARE_BOOL_OPERATOR(>)
DECLARE_BOOL_OPERATOR(<=)
DECLARE_BOOL_OPERATOR(>= )

typedef std::vector<Number> VecNumbers;
class Operator
{
public:
	Operator(int nOp = 0, int preced = 0, const char *name = "", const char *desc = "")
		: m_nNumberOfOperands(nOp), m_iPrecedence(preced), mName(name), mDesc(desc) {}
    virtual ~Operator() {}
	int GetNumberOfOperands() const { return m_nNumberOfOperands; }
	virtual Number Operate(const VecNumbers &numbers) const { return Number(""); }
	virtual const char *Name() const { return mName; }
	virtual const char *Desc() const { return mDesc && *mDesc ? mDesc : mName; }
	// http://en.cppreference.com/w/cpp/language/operator_precedence
	virtual unsigned Precendence() const { return m_iPrecedence; }
private:
	int m_nNumberOfOperands, m_iPrecedence;
	const char *mName, *mDesc;
};
class OperatorManager
{
public:
	static OperatorManager& GetInstance();
	const Operator* GetOperator(const char *str, bool bUnary = false);
	const Operator* GetNoOperator() const { return &mNoOp; }
	std::string GetDescription();
	~OperatorManager();
private:
	OperatorManager() {}
	void Init();
	std::vector<Operator*> mOperators;
	Operator mNoOp;
};


class Calculator {
public:
	~Calculator();
    Number EvaluateExpression(const char *expression);
	const std::string& GetErrorString() const { return mErrorStr; }
private:
    std::string mErrorStr;
	class ExpressionEntity
	{
	public:
		ExpressionEntity(const Operator *op = NULL) : m_pOperator(op) {}
		ExpressionEntity(Number n) : mNumber(n), m_pOperator(NULL) {}
		bool IsOperator() const { return m_pOperator != NULL; }
		std::string GetAsString() const { return m_pOperator ? m_pOperator->Name() : mNumber.GetAsString(); }
		Number mNumber;
		const Operator *m_pOperator;
	};
	typedef std::vector<ExpressionEntity> VecExpressionEntity;
	bool InfixToPostFix(const std::string &inExpStr, VecExpressionEntity &outPostFixExpression);
	Number EvalPostFixExpression(const VecExpressionEntity &inPostFixExpression);
};

#endif /* Calculator_hpp */
