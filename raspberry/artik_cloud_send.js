var webSocketUrl = "wss://api.artik.cloud/v1.1/websocket?ack=true";
var device_id = "7b2a64626f8f4b5caac2a3fee8e4adc2";
var device_token = "e65c8e75d365462e8fdf6069685c146d";

var readline = require('readline');

var r = readline.createInterface({ 
	input:process.stdin, 
	output:process.stdout 
}); 

r.setPrompt('> ');
r.prompt();
r.on('line',function(line){
	if(line == '-1'){
		var send_data = parseFloat("-1");
		sendData(send_data);
		console.log("----exit----");
		r.close();
	}
});

r.on('close', function() {
	process.exit(); 
});


var GPIO = require('onoff').Gpio,
	button_right = new GPIO(5,'in','both'),
	button_down = new GPIO(6,'in','both'),
	button_up = new GPIO(13,'in','both'),
	button_left = new GPIO(19,'in','both');

var isWebSocketReady = false;
var ws = null;

var WebSocket = require('ws');

try{
	button_right.watch(function(err, state) {
		if(state == 1){
			var send_data = parseFloat("3");
			sendData(send_data);
			console.log("----- button right input -----");
		}
	});

	button_left.watch(function(err, state) {
		if(state == 1){
			var send_data = parseFloat("2");
			sendData(send_data);
			console.log("----- button left input -----");
		}
	});

	button_up.watch(function(err, state) {
		if(state == 1){
			var send_data = parseFloat("1");
			sendData(send_data);
			console.log("----- button up input -----");
		}
	});

	button_down.watch(function(err, state) {
		if(state == 1){
			var send_data = parseFloat("4");
			sendData(send_data);
			console.log("----- button down input -----");
		}
	});
}
catch(exception){
	var send_data = parseFloat("-1");
	sendData(send_data);
	console.log("----- button up close -----");
}


function getTimeMillis(){
    return parseInt(Date.now().toString());
}

/**
 * Create a /websocket device channel connection
 */
function start() {
    //Create the websocket connection
    isWebSocketReady = false;
    ws = new WebSocket(webSocketUrl);
    ws.on('open', function() {
        console.log("Websocket connection is open ....");
        register(); // 레지스터 함수 호출
    });

    ws.on('close', function() {
        console.log("Websocket connection is closed ....");
    });
}

/**
 * Sends a register message to the websocket and starts the message flooder
 */
function register(){
    console.log("Registering device on the websocket connection");
    try{
        var registerMessage = '{"type":"register", "sdid":"'+device_id+'", "Authorization":"bearer '+device_token+'", "cid":"'+getTimeMillis()+'"}';
        console.log('Sending register message ' + registerMessage + '\n');
        ws.send(registerMessage, {mask: true});
        isWebSocketReady = true;
    }
    catch (e) {
        console.error('Failed to register messages. Error in registering message: ' + e.toString());
    }

}

/**
 * Send one message to ARTIK Cloud
 */
function sendData(onFire){
    try{
        ts = ', "ts": '+getTimeMillis();
        var data = {
            "left_motor_speed": onFire
        };
        var payload = '{"sdid":"'+device_id+'"'+ts+', "data": '+JSON.stringify(data)+', "cid":"'+getTimeMillis()+'"}';
        console.log('Sending payload ' + payload);
        ws.send(payload, {mask: true});
    } catch (e) {
        console.error('Error in sending a message: ' + e.toString());
    }   
}

/**
 * All start here
 */
start(); // create websocket connection

