/*
 * CharLibManagerApp.java
 */

package charlibmanager;

import javax.swing.JOptionPane;
import manager.Manager;
import org.jdesktop.application.Application;
import org.jdesktop.application.SingleFrameApplication;

/**
 * The main class of the application.
 */
public class CharLibManagerApp extends SingleFrameApplication {

    /**
     * At startup create and show the main frame of the application.
     */
    @Override protected void startup() {
        CharLibManagerView view = new CharLibManagerView(this);        
        try {
            Manager.boundView(view);
            Manager.loadCurrLib();
        } catch (Exception ex) {
            JOptionPane.showMessageDialog(view.getComponent(), ex.getMessage());
        }
        
        show(view);
    }

    /**
     * This method is to initialize the specified window by injecting resources.
     * Windows shown in our application come fully initialized from the GUI
     * builder, so this additional configuration is not needed.
     */
    @Override protected void configureWindow(java.awt.Window root) {
    }

    /**
     * A convenient static getter for the application instance.
     * @return the instance of CharLibManagerApp
     */
    public static CharLibManagerApp getApplication() {
        return Application.getInstance(CharLibManagerApp.class);
    }

    /**
     * Main method launching the application.
     */
    public static void main(String[] args) {
        launch(CharLibManagerApp.class, args);
    }
}
