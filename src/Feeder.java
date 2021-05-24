public class Feeder extends Thread{
    String frame;
    Viewer viewer;
    GUI gui;

    //constructor
    Feeder (Viewer fp, String str, GUI gui) {
        viewer = fp;
        frame = str;
        this.gui = gui;
        start();
    }

    public void run()
    {
        //synchronized block
        synchronized(frame)
        {
            viewer.display(frame, this.gui);
        }
    }
}
