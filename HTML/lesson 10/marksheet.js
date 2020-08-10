function onSubjectAndMarksDoubleClick() {
    console.log(this.innerText, "clicked");
    var overlayInput = document.getElementById("overlayInput");
    var itemClickedRect = this.getBoundingClientRect();
    // overlayInput.style.left  = itemClickedRect.left;
    // overlayInput.style.top  = itemClickedRect.top;
    // overlayInput.style.right  = itemClickedRect.right;
    // overlayInput.style.bottom  = itemClickedRect.bottom;
    overlayInput.setAttribute("style",
        "position: absolute;" +
        "left:" + itemClickedRect.left + "px;" +
        "top:" + itemClickedRect.top + "px;" +
        "width:" + (itemClickedRect.width - 8) + "px;" +
        "height:" + (itemClickedRect.height - 6) + "px;"
    );
    overlayInput.value = this.innerText;
    overlayInput.style.display = 'block';
    overlayInput.focus();
}

function onMarksObtainChange() {
    updateTotalMarksObtained();
    updatePercentage();
    updateResult();
}

function updatePercentage() {
    var totalMaxMarks = Number(document.getElementById("totalMaxMarks").innerText);
    var totalMarksObtained = Number(document.getElementById("totalMarksObtained").innerText);
    var percentage = totalMarksObtained * 100 / totalMaxMarks;

    document.getElementById("percentage").innerText = percentage.toFixed(2);
}

function updateResult() {
    var percentage = Number(document.getElementById("percentage").innerText);
    var result = "";
    var resultTag = document.getElementById("result");
    resultTag.className = null;
    if (percentage >= 75)
        result = "Passed with Distinction";
    else if (percentage >= 60)
        result = "Passed with First Division";
    else if (percentage >= 45)
        result = "Passed with Second Division";
    else if (percentage >= 34)
        result = "Passed with Third Division";
    else {
        result = "FAILED";
        resultTag.className = "resultFailed";
    }
    resultTag.innerText = result;
}

function addSubject(subjectName, maxMarks) {
    var table = document.getElementById("subjects");
    // create row
    var tr = document.createElement("tr");
    table.appendChild(tr); // add to table

    // 1st column
    var td = document.createElement("td");
    td.innerText = subjectName; // text set
    td.ondblclick = onSubjectAndMarksDoubleClick;
    tr.appendChild(td); // add to row

    // 2 nd col
    td = document.createElement("td");
    td.innerText = maxMarks; // text set
    td.className = "maximumMarks";
    td.ondblclick = onSubjectAndMarksDoubleClick;
    tr.appendChild(td); // add to row

    // 3 rd col - marks obtained
    td = document.createElement("td");
    var input = document.createElement("input");
    input.oninput = onMarksObtainChange;
    input.type = "number";
    input.min = 0;
    //    input.max = maxMarks;

    input.className = "marksObtained";
    td.appendChild(input);
    tr.appendChild(td); // add to row
}

function updateTotalMarksObtained() {
    var totalMarksObtained = document.getElementsByClassName("marksObtained");
    var maximumMarksHTMLTags = document.getElementsByClassName("maximumMarks");
    var totalMarks = 0;

    for (var i = 0; i < totalMarksObtained.length; ++i) {
        var maxMarks = Number(maximumMarksHTMLTags[i].innerText);
        var marksObtained = checkAndUpdateMarksObtained(totalMarksObtained[i], maxMarks);

        totalMarks += marksObtained;
    }

    // update in html
    document.getElementById("totalMarksObtained").innerText = Number.isInteger(totalMarks) ? totalMarks : totalMarks.toFixed(2);
}

function checkAndUpdateMarksObtained(marksObtainedElement, maxMarks) {
    var marksObtained = marksObtainedElement.valueAsNumber;
    if (marksObtained < 0 || isNaN(marksObtained)) {
        marksObtained = 0;
        marksObtainedElement.value = "";
    }
    if (marksObtained.toFixed(2) != marksObtained) {
        marksObtained = Number(marksObtained.toFixed(2));
        marksObtainedElement.value = marksObtained;
    }
    if (marksObtained > maxMarks) {
        marksObtained = maxMarks;
        marksObtainedElement.value = marksObtained;
    }

    return marksObtained;
}

function updateTotalMarks() {
    var maximumMarksHTMLTags = document.getElementsByClassName("maximumMarks");
    var totalMaxMarks = 0;

    for (var i = 0; i < maximumMarksHTMLTags.length; ++i) {
        var maxMarks = Number(maximumMarksHTMLTags[i].innerText);
        totalMaxMarks += maxMarks;
    }

    // update in html
    document.getElementById("totalMaxMarks").innerText = totalMaxMarks;
}

function loadDefaultSujects() {
    var subjects = ["English", "Mathematics", "Science", "Hindi", "Social Science"];
    var maxMarks = [100, 70, 90, 100, 50];
    for (var i = 0; i < subjects.length; ++i)
        addSubject(subjects[i], maxMarks[i]);

    updateTotalMarks();
}

function onHtmlLoad() {
    loadDefaultSujects();
}

function onInputFocusOut()
{
    var overlayInput = document.getElementById("overlayInput");

    overlayInput.style.display = "none";
}