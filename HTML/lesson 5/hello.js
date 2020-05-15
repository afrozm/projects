
function CalculateAge(startYear, endYear) {
    if (startYear > endYear)
        var age = startYear - endYear;
    else
        var age = endYear - startYear;
    
    return age;
}

function printResult(result) {
    console.log("result=",result);
}

var myAge = CalculateAge(2020, 1982); // year1=2020, year2=1982
printResult(myAge); 


var rizAge = CalculateAge(1987, 2020); // year1=1987, year2=2020

console.log("Riz age is", rizAge); 

// a, b, c
// a + b - c

// == -> equal to
// != not equal
// ! not
// || or - false || false -> false otherwise true
// && and - true && true -> true otherwise false