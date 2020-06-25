function checkPrime() {
    var number = Number(document.getElementById("number").value);
    var isPrime = true;
    for (var I = 2; I < number; ++I) {
        var remainder = number % I;
        if (remainder == 0) {
            isPrime = false;
            break;
        }
    }
    var result = "Number " + number + " is ";
    if (!isPrime)
        result += "not ";
    result += "prime number";
    if (!isPrime)
        result += " as it is divisible by " + I;
    document.getElementById("result").innerText = result;
}