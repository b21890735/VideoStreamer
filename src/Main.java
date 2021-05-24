import java.io.*;
import java.net.Socket;
import java.util.Arrays;

public class Main {
    public static void main(String[] args) throws Exception {
        try {
            String address = args[1];
            int port = Integer.parseInt(args[3]);
            char channelID = args[5].charAt(0);

            Socket s = new Socket(address, port);
            DataInputStream din = new DataInputStream(s.getInputStream());
            DataOutputStream dout = new DataOutputStream(s.getOutputStream());
            BufferedReader br = new BufferedReader(new InputStreamReader(din));
            String totalFrame = "";

            GUI gui = null;

            try {
                gui = new GUI("ASCII VIDEO STREAMER");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            Viewer viewer = new Viewer();

            // Reading data using readLine
            String str = "", str2 = "";
            str = br.readLine();
            System.out.println(str);

            dout.write(channelID);
            dout.flush();

            while (true) {
                int counter = 0;
               totalFrame = "";
                while (counter < 14) {
                    str = br.readLine();
                    if (counter != 0) {
                        totalFrame = totalFrame.concat(str + '\n');
                    }
                    counter++;
                }

                Feeder feeder = new Feeder(viewer, totalFrame, gui);

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