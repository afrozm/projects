function testArray() {

    var pizzaParts = ['pepperoni', 'onion', 'bacon'];
    var emptyArray = [];

    var firstItem = pizzaParts[0];
    var secondItem = pizzaParts[1];
    pizzaParts[2] = "Cheese";

    var pizzaPartsLength = pizzaParts.length;
    var emptyArrayLength = emptyArray.length;

    var toppings = ['Olives', 'Chillies'];
    var myPizzaArray = pizzaParts.concat(toppings);
    var myPizzaArrayLength = myPizzaArray.length;

    var indexOfOnion = pizzaParts.indexOf('onion');
    var indexOfBacon = pizzaParts.indexOf('bacon');

    var joinString = pizzaParts.join();

    pizzaPartsLength = pizzaParts.push("bacon");
    pizzaPartsLength = pizzaParts.push("mushroom");
    var pizzaPartsLastElement = pizzaParts.pop();
}

function highestNumber()
{
    var numbers = [5 , 3, 2, 6, 4, 9];
            //     0   1  2  3  4  5
    var highestNum = numbers[0];
    for (var i=1; i < numbers.length; ++i) {
        var nextNumber = numbers[i];
        if (highestNum < nextNumber) // if (nextNumber > highestNum)
            highestNum = nextNumber;
    }
}