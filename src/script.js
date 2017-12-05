Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);
var word2;
var defn;
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    console.log('Received message: '+ JSON.stringify(e.payload));
    console.log('check data ' + e.data[0]);
    word2 = e.data['KEY_DEFINITION'].toLowerCase();
    getDefinition();
  }                     
);
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getDefinition() {
  var url = "https://glosbe.com/gapi/translate?from=eng&dest=eng&format=json&phrase="+word2+"&pretty=true";
  xhrRequest(url, 'GET',
      function(responseText) {
        var json = JSON.parse(responseText);
        var results = json.tuc;
         if (results && results.length > 0 && results[0].meanings && results[0].meanings.length > 0) {
           // we have at least 1 definition
          var definitions = results[0].meanings;
          //var defn = definitions[0].text;
           var defn = "";
           var counter = 0;
           for(var i=0;i<3;i++) {
             console.log("in for loop" + defn);
             if(definitions[i].text.length > 256){
               continue;
             }
             counter++;
             defn += ((counter)+". "+definitions[i].text+"\n\n");
           }
           var dictionary = {
             'KEY_DEFINITION':defn
           };
           Pebble.sendAppMessage(dictionary, function() {
                console.log("it worked!!" + defn);
         }, function() {
           console.log("it failed.");
         });
          console.log('Definition: '+defn);
        }
      }
   );
}
