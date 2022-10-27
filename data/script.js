//Fonction de commande Javascript avec Jquery

$(document).ready(function(){
    $("#saveButton").click(function(){
        var valeur = document.querySelector('input[name="networktype"]:checked').value;
        if (confirm("Vous confirmer la sauvegarde et le redémarrage ?"))
        {
            $.post("receiveData",{
            hostname: $("#inputhostname").val(),
            wifiname: $("#inputwifiname").val(),
            wifipassword: $("#inputwifipassword").val(),
            //networktype: document.getElementsByName("networktype").,
            networktype: valeur,
            httpenable: document.getElementById("HTTPcheckboxId").checked,
            rtspenable : document.getElementById("RTSPcheckboxId").checked,
            portrtsp: $("#inputportrtsp").val()
            });
        };
    });
});


function reinitButton() {
    var xhttp = new XMLHttpRequest();
    if (confirm("Confirmer la reinitialisation ?\n (Attention les données de connection seront perdues)")) {
        xhttp.open("GET", "reinit", true);
        xhttp.send();
        };
};

function restartButton() {
    var xhttp = new XMLHttpRequest();
        xhttp.open("GET", "restart", true);
        xhttp.send();
};

function getData()
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
        document.getElementById("displayversion").innerHTML = this.responseXML.getElementsByTagName('version')[0].childNodes[0].nodeValue; 
        document.getElementById("displayipadresse").innerHTML = this.responseXML.getElementsByTagName('ipadresse')[0].childNodes[0].nodeValue; 
        document.getElementById("displayssid").innerHTML = this.responseXML.getElementsByTagName('namessid')[0].childNodes[0].nodeValue; 
        document.getElementById("displayhostname").innerHTML = this.responseXML.getElementsByTagName('hostname')[0].childNodes[0].nodeValue;     
        document.getElementById("displayportrtsp").innerHTML = this.responseXML.getElementsByTagName('portrtsp')[0].childNodes[0].nodeValue;     
        if(this.responseXML.getElementsByTagName('wifimode')[0].childNodes[0].nodeValue == "STA")
        {
            AccessPointId.checked=true;
        };
        if(this.responseXML.getElementsByTagName('wifimode')[0].childNodes[0].nodeValue == "AP")
        {
            AccessWifiId.checked=true;
        };
        if(this.responseXML.getElementsByTagName('http_enable')[0].childNodes[0].nodeValue == "1")
        {
            HTTPcheckboxId.checked=true;
        };
        if(this.responseXML.getElementsByTagName('rtsp_enable')[0].childNodes[0].nodeValue == "1")
        {
            RTSPcheckboxId.checked=true;
        };
        //var element=document.getElementById("inputhostname").innerHTML;

        }
    };
    xhttp.open("GET", "getData", true);
    xhttp.send();
};

//function saveButton() {
//    var xhttp = new XMLHttpRequest();
//    if (confirm("Confirmer la sauvegarde ?")) {
//        xhttp.open("GET", "save", true);
//        xhttp.send();
//        };
//};

// Fonctions de développement
//setInterval(function getData2()
//{
//    var xhttp = new XMLHttpRequest();
//    xhttp.onreadystatechange = function()
//    {                                                                               
//        if(this.readyState == 4 && this.status == 200)
//        {
//            document.getElementById("inputssid").innerHTML = this.responseText;
//        }
//    };
//    xhttp.open("GET", "envoid1", true);
//    xhttp.send();
//}, 2000);

//setInterval(function getInputTest() {
//    var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() {
//    if (this.readyState == 4 && this.status == 200)
//    {
//    document.getElementById("displayhostname").innerHTML = this.responseXML.getElementsByTagName('button1')[0].childNodes[0].nodeValue; document.getElementById("input2").innerHTML = this.responseXML.getElementsByTagName('button2')[0].childNodes[0].nodeValue; }
//    };
//    xhttp.open("GET", "getInputTest", true);
//    xhttp.send();
//}, 2000);

//$(document).ready(function(){
//    $("#appliquer").click(function(){
//        var valeur = $("#choixDelayLed").val();
//        $.post("delayLed",{
//            valeurDelayLed: valeur
//        });
//    });
//});