import java.io.*;
import java.net.Socket;

public class Main {
    public static void main(String[] args) throws Exception {
        try {
            String address = args[0];
            int port = Integer.parseInt(args[1]);
            char channelID = args[2].charAt(0);

            Socket s = new Socket(address, port);
            DataInputStream din = new DataInputStream(s.getInputStream());
            DataOutputStream dout = new DataOutputStream(s.getOutputStream());
            BufferedReader br = new BufferedReader(new InputStreamReader(din));
            BufferedReader channel_reader = new BufferedReader(new InputStreamReader(System.in));


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
            //int channel_num = channel_reader.read();
            System.out.println(str);

            dout.write('2');
            dout.flush();

            while (true) {
                int counter = 0;
                String total_frame = "";
                while (counter < 14) {
                    str = br.readLine();
                    if (counter != 0) {
                        total_frame = total_frame.concat(str + '\n');
                    }
                    counter++;
                }
                System.out.println(total_frame);

                Feeder feeder = new Feeder(viewer, total_frame, gui);

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