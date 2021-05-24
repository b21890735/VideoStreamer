import javax.swing.*;
import java.awt.*;

import static javax.swing.JFrame.EXIT_ON_CLOSE;

public class GUI {
    JTextArea area = new JTextArea("ASCII VIDEO STREAMER");
    JFrame f= new JFrame();

    public JTextArea getArea() {
        return area;
    }

    public void setArea(JTextArea area) {
        this.area = area;
    }

    GUI(String buffer) throws InterruptedException {
        //set size, layout etc
        this.f.setSize(500,300);
        this.f.setLayout(null);
        this.f.setVisible(true);

        //set font to display frames properly
        this.area.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));

        //when user clicks close button, gui will be closed
        this.f.setDefaultCloseOperation(EXIT_ON_CLOSE);
        this.area.setText(buffer);
        this.area.setBounds(0,0, 500,300);
        this.f.add(this.area);

        //sleep thread for 50 ms in order to achieve 20 frame rate
        Thread.sleep(50);
    }
}
