var socket;
var login = {};

$(initLogin);

function initLogin() {
	// load content for login page and event event listener for button
	$("#content").load("content/loginContent.html", function() {
		$("#begin").click(initChatroom);
	});
}

function initChatroom() {
	// get settings from login page
	login.server = $("#server").val();
	login.port = $("#port").val();
	login.password = $("#password").val();
	login.key = $("#key").val();
	login.nickname = $("#nickname").val();

	// load chat page and add listeners for submit button
	$("#content").load("content/chatboxContent.html", function() {
		$('#form').submit(function(e) {
			e.preventDefault();
			submitHandler(this);
		});
	});

	// connect to server and attempt user validation
	socket = new WebSocket("ws://" + login.server + ":" + login.port);
	// socket.send(login.password);

	// add callback functions for networking
	socket.onopen = function (event) {
		socket.send(login.password);
	}
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