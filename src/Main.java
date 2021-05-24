import java.io.*;
import java.net.Socket;

public class Main {
    public static void main(String[] args) throws Exception {
        try {
            //get arguments from command line
            String address = args[1];
            int port = Integer.parseInt(args[3]);
            char channelID = args[5].charAt(0);

            //create socket on the specified address and port
            Socket s = new Socket(address, port);

            //create input stream to read data from socket
            DataInputStream din = new DataInputStream(s.getInputStream());

            //create output stream to write data to stream
            DataOutputStream dout = new DataOutputStream(s.getOutputStream());

            //buffered reader for socket input
            BufferedReader br = new BufferedReader(new InputStreamReader(din));

            //create GUI object
            GUI gui = null;

            try {
                gui = new GUI("ASCII VIDEO STREAMER");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            // Reading data using readLine
            String str = "", str2 = "";
            str = br.readLine();
            System.out.println(str);

            //send channelID to server via socket
            //so that server send related channel data to client
            dout.write(channelID);
            dout.flush();

            final FeederViewer.PC pc = new FeederViewer.PC(dout, br, gui);

            // Create feeder thread
            Thread feeder = new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        pc.produce();
                    }
                    catch (InterruptedException | IOException e) {
                        e.printStackTrace();
                    }
                }
            });

            // Create viewer thread
            Thread viewer = new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        pc.consume();
                    }
                    catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            });

            // Start both threads
            feeder.start();
            viewer.start();

            // feeder finishes before viewer
            feeder.join();
            viewer.join();

            //dout.close();
            //s.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}