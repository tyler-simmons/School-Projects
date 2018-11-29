var socket;
var login = {
	server: "localhost",
	port: "8887",
	password: "password",
	key: "key",
	nickname: "nickname"

}

$(initChatroom(login));

function initChatroom(login) {

	// connect to server and attempt user validation
	socket = new WebSocket("ws://" + login.server + ":" + login.port);
	// socket.send(login.password);

	// add callback functions for UI and networking
	$('#form').submit(function(e) {
		e.preventDefault();
		submitHandler(this);
	});
	socket.onmessage = function (event) {
		addMessage(JSON.parse(sjcl.decrypt(login.key,event.data)));
	}
	socket.onclose = function (event) {
		alert("Disconnected! Check server password and network connection.")
	}
}

function submitHandler(form) {
	var message = {
		name: login.nickname,
		content: form["msg"].value
	}
	form["msg"].value = "";
	socket.send(sjcl.encrypt(login.key,JSON.stringify(message)));
}

function addMessage(message) {
	$('#content tr:first').before('<tr><td>' + message.name + '</td><td>' + message.content + '</td></tr>');
}