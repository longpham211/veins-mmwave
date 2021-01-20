import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

public class AddOverheadForIeee80211ad {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	
	public static void main(String[] args) throws IOException {
		String referenceFilePath = args[0];
		String outputFilePath = args[1];
		
		
		BufferedReader br = new BufferedReader(new FileReader(referenceFilePath));
		File outputFile = new File(outputFilePath);
		Writer writer = new FileWriter(outputFile, true);
		
		String line;
		String[] values;
		
		while((line = br.readLine()) != null)  {
			values = line.split(USE_DELIMITER);
			if(values.length > 1) {
				writer.append(values[0])
					.append(USE_DELIMITER)
					.append("0")
					.append("\n");
			}
			else
				writer.append(values[0])
					  .append("\n");
		}
		writer.close();
	}
}
