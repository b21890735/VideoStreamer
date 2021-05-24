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
            String totalFrame = "";

            //create GUI object
            GUI gui = null;

            try {
                gui = new GUI("ASCII VIDEO STREAMER");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            //viewer thread
            Viewer viewer = new Viewer();

            // Reading data using readLine
            String str = "", str2 = "";
            str = br.readLine();
            System.out.println(str);

            //send channelID to server via socket
            //so that server send related channel data to client
            dout.write(channelID);
            dout.flush();

            while (true) {
                int counter = 0;
                totalFrame = "";
                //read entire frame and send it to feeder
                while (counter < 14) {
                    str = br.readLine();
                    if (counter != 0) {
                        totalFrame = totalFrame.concat(str + '\n');
                    }
                    counter++;
                }

                //feeder thread update
                Feeder feeder = new Feeder(viewer, totalFrame, gui);

                //send data to server via socket
                //so that it continues to send data
                dout.write('\n');
                dout.flush();
            }

            //dout.close();
            //s.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}