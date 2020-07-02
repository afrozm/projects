function addSubject(subjectName, maxMarks)
{
    var table = document.getElementById("subjects");
    // create row
    var tr = document.createElement("tr");
    table.appendChild(tr); // add to table

    // 1st column
    var td = document.createElement("td");
    td.innerText = subjectName; // text set
    tr.appendChild(td); // add to row

    // 2 nd col
    td = document.createElement("td");
    td.innerText = maxMarks; // text set
    tr.appendChild(td); // add to row

    // 3 rd col - marks obtained
    td = document.createElement("td");
    var input = document.createElement("input");
    td.appendChild(input);
    tr.appendChild(td); // add to row
}

function loadDefaultSujects()
{
    var subjects = ["English", "Mathematics", "Science", "Hindi", "Social Science"];
    for (var i=0; i<subjects.length; ++i)
        addSubject(subjects[i], 100);
}

function onHtmlLoad() {
    loadDefaultSujects();
}