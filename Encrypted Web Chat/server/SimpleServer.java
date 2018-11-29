import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.HashMap;

import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.server.WebSocketServer;

public class SimpleServer extends WebSocketServer {

        private String serverPassword;
        private final String encryptionPrefix = "\"iv\":\"";
        private HashMap<WebSocket,Boolean> accessController;
        
	
	public SimpleServer(InetSocketAddress address) {
		super(address);
                this.accessController = new HashMap<>();
                this.serverPassword = "password";
	}

	@Override
	public void onOpen(WebSocket conn, ClientHandshake handshake) {
                System.out.println("Server: new connection @"+ conn.getRemoteSocketAddress());
        }

	@Override
	public void onClose(WebSocket conn, int code, String reason, boolean remote) {
		System.out.println("CLOSED " + conn.getRemoteSocketAddress() + " WITH EXIT CODE " + code + " additional info: " + reason);
	}

	@Override
	public void onMessage(WebSocket conn, String message) {
		//System.out.println("Client @ " + conn.getRemoteSocketAddress() +  ": " + message);
                String comparison = message.substring(1);
                /*
                 * Test to see if the message is enrypted per jscl standard or sent as plaintext
                 */
                if (!comparison.startsWith(encryptionPrefix)){
                    /*
                     * Message is plain text, attempt to validate with server
                     */
                    System.out.println("Server: new client attempting connection @ " + conn.getRemoteSocketAddress());
                    if (message.equals(serverPassword)){
                        System.out.println("Server: connection from client @ " + conn.getRemoteSocketAddress() + " accepted");
                        accessController.put(conn, Boolean.TRUE);
                    } else {
                        /*
                         * Incorrect pasword, notify client and close connection
                         */
                        System.out.println("Server: incorrect password from client @ " + conn.getRemoteSocketAddress());
                        System.out.println("Terminating connection");
                        conn.send("Message from server: incorrect server password");
                        conn.close();
                    }
                } else {
                    /*
                     * Moving forward on precondition that the message is in the encrypted form
                     * First step is the check authentication status against the accessController
                     */
                    if (accessController.get(conn).equals(Boolean.TRUE)){
                        /*
                         * Now the message can be broadcast to the ramaining clients
                         */
                        System.out.println("Server: new message from client @ " + conn.getRemoteSocketAddress());
                        broadcast(message);
                    } else {
                        /*
                         * Security redundancy check
                         */
                        conn.close();
                        System.out.println("Debug: bad hashkey val");
                    }
                    
                    
                }
                
                
                
//                broadcast(message);
	}

	@Override
	public void onError(WebSocket conn, Exception ex) {
		System.err.println("ERROR ON CONNECTION: " + conn.getRemoteSocketAddress()  + ":" + ex);
	}
	
	@Override
	public void onStart() {
		System.out.println("Server: Server started successfully");
		try {
			URL ipURL = new URL("http://checkip.amazonaws.com");
			BufferedReader in = new BufferedReader(new InputStreamReader(ipURL.openStream()));
			System.out.println("Server: Listening on ip: " + in.readLine());
			in.close();
		} catch (Exception e) {
			System.out.println("Server: Error retrieving server broadcast ip");
		}
		
	}


	public static void main(String[] args) {
		int port = 8887;
		WebSocketServer server = new SimpleServer(new InetSocketAddress(port));
		server.run();	
	}
}