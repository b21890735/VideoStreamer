import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.LinkedList;

public class FeederViewer {
    // This class has a buffer, producer (adds items to buffer
    // and consumer (removes items).
    public static class PC {

        // Create a buffer shared by producer and consumer
        // Size of buffer is 2.
        LinkedList<String> buffer = new LinkedList<>();
        int capacity = 2;

        //create output stream to write data to stream
        DataOutputStream dout;

        //buffered reader for socket input
        BufferedReader br;

        GUI gui;

        PC(DataOutputStream dout, BufferedReader br, GUI gui){
            this.dout = dout;
            this.br = br;
            this.gui = gui;
        }

        public void produce() throws InterruptedException, IOException {
            while (true) {
                synchronized (this) {
                    while (buffer.size() == capacity){
                        System.out.println("BUFFER IS FULL");
                        wait();
                    }

                    int counter = 0;
                    String totalFrame = "";
                    //read entire frame and send it to feeder
                    while (counter < 14) {
                        String str = br.readLine();
                        if (counter != 0) {
                            totalFrame = totalFrame.concat(str + '\n');
                        }
                        counter++;
                    }

                    //add data read from socket to buffer
                    buffer.add(totalFrame);

                    // wake up consumer thread
                    notify();

                    //send data to server via socket
                    //so that it continues to send data
                    dout.write('\n');
                    dout.flush();

                }
            }

        }

        // Function called by consumer thread
        public void consume() throws InterruptedException {
            while (true) {
                synchronized (this) {
                    // consumer thread waits while buffer
                    // is empty
                    while (buffer.size() == 0){
                        System.out.println("BUFFER IS EMPTY");
                        wait();
                    }

                    // to retrive the first job in the buffer
                    String val = buffer.removeFirst();

                    // wake up feeder thread
                    notify();

                    display(val);
                }
            }
        }

        public void display(String frame) {
            gui.area.setText(frame);
            gui.f.add(gui.getArea());

            try {
                //sleep thread for 50 ms in order to achieve 20 frame rate
                Thread.sleep(50);
            } catch (InterruptedException var4) {
                var4.printStackTrace();
            }

        }
    }
}

