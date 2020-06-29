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

function findInList(list,  object) {
    for (var index=0; index <list.length; ++index) {
        var objectInList = list[index];
        if (object == objectInList)
            return index;
    }
    // no match found return is -1 number
    return -1;
}

function secondHighestNumber()
{
    var numbers = [5 , 3, 2, 6, 4, 9]; // count 6
    var strings = ['Afroz', 'Rizwan', 'Shahnaaz', 'Anjali'];

        //          0   1  2  3  4  5
    var myIndex = findInList(numbers, 6);
    myIndex = findInList(numbers, 7);
    var index = numbers.indexOf(6) // search in list/array
    index = numbers.indexOf(7);



    numbers.sort();
    numbers.reverse();
    var smallestNumber = numbers[0];
    var secondSmallestNumber = numbers[1]
    var highestNum = numbers[numbers.length - 1];
    var secondHighestNumber = numbers[numbers.length - 2];
}