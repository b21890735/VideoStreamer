
public class Viewer extends Thread {
    public Viewer() {
    }

    public void display(String var1, GUI var2) {
        var2.area.setText(var1);
        var2.f.add(var2.getArea());

        try {
            //sleep thread for 50 ms in order to achieve 20 frame rate
            Thread.sleep(50);
        } catch (InterruptedException var4) {
            var4.printStackTrace();
        }

    }
}
