//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by FernFlower decompiler)
//

public class Viewer extends Thread {
    public Viewer() {
    }

    public void display(String var1, GUI var2) {
        var2.area.setText(var1);
        var2.f.add(var2.getArea());

        try {
            Thread.sleep(50);
        } catch (InterruptedException var4) {
            var4.printStackTrace();
        }

    }
}
