import javax.xml.transform.stream.StreamSource;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Pargrepmon {

    private ConcurrentLinkedQueue<String> searchStrings = new ConcurrentLinkedQueue<>();
    private List<String> lines = new ArrayList<>();
    final private HashMap<String,Integer> output = new HashMap<>();
    public void run(String searchStringFilePath, String inputDataFilePath)
    {
        Path searchFile = Paths.get(searchStringFilePath);
        Path inputFile = Paths.get(inputDataFilePath);

        try {
            searchStrings.addAll(Files.readAllLines(searchFile, Charset.defaultCharset()));
            lines.addAll(Files.readAllLines(inputFile, Charset.defaultCharset()));
        } catch (IOException e) {
            e.printStackTrace();
        }
        ExecutorService es = Executors.newCachedThreadPool();
        for(int i=0;i<searchStrings.size();i++)
            es.execute(this::lookForIt);
        es.shutdown();
        try {
            boolean finished = es.awaitTermination(2, TimeUnit.MINUTES);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
       output.entrySet().stream().map(x-> x.getKey()+";"+x.getValue()).forEach(System.out::println);
    }


    public static void main (String[] args) {
        if (args.length != 2) {
            System.err.println("[Error] Usage: <program> <search string file path> <input data file path>");
            return;
        }
        String searchStringFilePath = args[0];
        String inputDataFilePath = args[1];
        new Pargrepmon().run(searchStringFilePath,inputDataFilePath);
    }

    public void lookForIt()
    {
        var sString = searchStrings.poll();
        if (sString == null)
            return;
        for(var line: this.lines)
        {
            if(line.contains(sString))
                add2Output(sString);
        }
    }
    private void add2Output(String value)  {
            synchronized (this.output) {
                try {
                    output.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if(!output.containsKey(value))
                   output.put(value,1);
               else
                   output.replace(value,output.get(value)+1);
                output.notify();
            }


    }
}
