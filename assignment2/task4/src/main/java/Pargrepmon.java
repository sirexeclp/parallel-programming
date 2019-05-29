import java.util.LinkedList;
import java.util.List;

public class Pargrepmon {
    public static void main (String[] args) {
        if (args.length != 2) {
            System.err.println("[Error] Usage: <program> <search string file path> <input data file path>");
        }

        String searchStringFilePath = args[0];
        String inputDataFilePath = args[1];

        List<String> searchStrings = new LinkedList<String>();
        List<String> inputLines = new LinkedList<String>();
    }
}
