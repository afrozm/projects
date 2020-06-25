function swtichArticle(itemClicked)
{
    console.log(itemClicked.innerText, "clicked");
    
    var articleList = document.getElementsByTagName("article");

    for (var i=0; i < articleList.length; ++i) {
        var article = articleList[ i ]; // get element
        article.style.display = 'none'; // hide
    }
    
    document.getElementById(itemClicked.innerText).style.display = 'block'; // show the clicked article
}