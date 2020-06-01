console.log("Welcome to Calculator");
function Add() {
    var number1 = Number(document.getElementById("number1").value);
    var number2 = Number(document.getElementById("number2").value);
    var result = 0;
    result = number1 + number2;
    console.log(number1, number2, "result", result);
    document.getElementById("result").innerText = result;
}
function Subtract() {
    var number1 = Number(document.getElementById("number1").value);
    var number2 = Number(document.getElementById("number2").value);
    var result = 0;
    result = number1 - number2;
    console.log(number1, number2, "result", result);
    document.getElementById("result").innerText = result;
}
function Multiply() {
    var number1 = Number(document.getElementById("number1").value);
    var number2 = Number(document.getElementById("number2").value);
    var result = 0;
    result = number1 * number2;
    console.log(number1, number2, "result", result);
    document.getElementById("result").innerText = result;
}
function Divide() {
    var number1 = Number(document.getElementById("number1").value);
    var number2 = Number(document.getElementById("number2").value);
    var result = 0;
    result = number1 / number2;
    console.log(number1, number2, "result", result);
    document.getElementById("result").innerText = result;
}
function Table(number1, number2) {
    var i=1;
    var result = '';
    while (i <= number2) {
        // 5 x 3 = 15
        result += number1 + ' x ' + i + ' = ' + number1 * i;
        result += '<br>';
        ++i;
    }
    return result;
}
function Calculate(buttonPressed) {
    var number1 = Number(document.getElementById("number1").value);
    var number2 = Number(document.getElementById("number2").value);
    var result = 0;

    switch (buttonPressed) {
        case '+':
            result = number1 + number2;
            break;
        case '-':
            result = number1 - number2;
            break;
        case '*':
            result = number1 * number2;
            break;
        case '/':
            result = number1 / number2;
            break;
        case 'Table':
            result = Table(number1, number2);
            break
    }
    console.log(number1, buttonPressed, number2, "=", result);
    document.getElementById("result").innerHTML = result;
}