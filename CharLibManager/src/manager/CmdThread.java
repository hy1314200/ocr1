/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package manager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import javax.swing.JTextArea;

/**
 *
 * @author liuyi
 */
public 
    
class CmdThread implements Runnable {

    private String cmd;
    
    private JTextArea display;

    public CmdThread(String cmd, JTextArea display) {
        this.cmd = cmd;
        this.display = display;
    }

    public void run() {
        try {
            Process process = Runtime.getRuntime().exec(cmd);

            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String buffer;
            boolean first = true;
            while ((buffer = reader.readLine()) != null) {
                if (first) {
                    display.setText("");
                    first = false;
                }

                display.append(buffer + "\n");
                display.setCaretPosition(display.getText().length());
            }
            reader.close();
        } catch (IOException ex) {
            display.setText("ocr.exe 执行出错");
        }
    }
}