public class Feeder extends Thread{
    String frame;
    Viewer viewer;
    GUI gui;

    Feeder (Viewer fp, String str, GUI gui) {
        viewer = fp;
        frame = str;
        this.gui = gui;
        start();
    }

    public void run()
    {
        synchronized(frame)      //Synchronized block
        {
            viewer.display(frame, this.gui);
        }
    }
}
