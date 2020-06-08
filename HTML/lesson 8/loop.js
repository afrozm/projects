function guessTheNumber()
{

    var inputNumber = Number(document.getElementById("number").value);
    var result = "cannot guess it";
    for (var i=1; i<=100; ++i) {

        if (i == inputNumber) { // == equal to 20==20
            result = i + "is the number";
            break;
        }


    }
    document.getElementById("result").innerText = result;
}

// 1+2+3+4+5+6+7+8+9+10=55
// 1..100 = 5050

function printSum() {
    var inputNumber = Number(document.getElementById("number").value);
    var result = "sum of 1 to " + inputNumber + " is";

    /// calc sum of 1 to inputNumber

    document.getElementById("result").innerText = result;
}