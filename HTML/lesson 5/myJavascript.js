function calculateResult() {
    var number1 = document.getElementById("number1").value;
    var number2 = document.getElementById("number2").value;

    var result = Number(number1) + Number(number2);

    //console.log("calculateResult called", number1, number2, result, "is the result");

    
//number1 + "  " + number2 + " are same age"

    document.getElementById("result").innerText = result;

}