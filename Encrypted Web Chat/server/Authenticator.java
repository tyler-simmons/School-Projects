import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.server.WebSocketServer;

public class Authenticator {
	
//	var user = {
//			name: "Person",
//			password: "password",
//			key: "key"
//		}
	
	private String serverPassword;
	
	
	//default setting for password is password
	public Authenticator() {
		this.serverPassword = "password";
	}
	
	//to be used in Server class
	public void setPassword(String input) {
		this.serverPassword = input;
	}
	
	public boolean authenticate(String password) {
		return password.equals(serverPassword);
	}
	
	
}