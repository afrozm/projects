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
function Devide() {
    var number1 = Number(document.getElementById("number1").value);
    var number2 = Number(document.getElementById("number2").value);
    var result = 0;
    result = number1 / number2;
    console.log(number1, number2, "result", result);
    document.getElementById("result").innerText = result;
}
