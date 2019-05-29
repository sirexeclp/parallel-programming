import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;
import java.util.ArrayList;

public class Pargrepmon {
    public static void main (String[] args) {
        if (args.length != 2) {
            System.err.println("[Error] Usage: <program> <search string file path> <input data file path>");
        }

        String searchStringFilePath = args[0];
        String inputDataFilePath = args[1];

        Path searchFile = Paths.get(searchStringFilePath);
        List<String> searchStrings = new ArrayList<>();
        try {
            searchStrings = Files.readAllLines(searchFile, Charset.defaultCharset());
        } catch (IOException e) {
            e.printStackTrace();
        }

        Path inputFile = Paths.get(inputDataFilePath);
        List<String> lines = new ArrayList<>();
        try {
            lines = Files.readAllLines(inputFile, Charset.defaultCharset());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
