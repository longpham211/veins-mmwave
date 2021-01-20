import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map.Entry;
import java.util.Scanner;
import java.util.TreeMap;


public class ProcessCSVFile {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	public static double GIGA = 1e9;
	public static double MEGA = 1e6;
	public static double KILO = 1e3;
	
	public static void main(String[] args) throws FileNotFoundException, IOException {
		String filePath = args[0];
		int cummulateInterval = Integer.parseInt(args[1]); 
		String convertDataString = args[2];		
		double convertData = GIGA;
		if(convertDataString.equals("K"))
			convertData = KILO;
		else if(convertDataString.equals("M"))
			convertData = MEGA;
		else if(convertDataString.equals("G"))
			convertData = GIGA;

		TreeMap<Integer, TreeMap<Integer, Double>> cummulateReceiveDataMapByTime = new TreeMap<>(); 
		
		List<List<String>> records = new ArrayList<>();
		BufferedReader br = new BufferedReader(new FileReader(filePath));
	    String line;
	    line = br.readLine(); // read the first line
	    if(line != null) {
	        String[] values = line.split(USE_DELIMITER);
	        records.add(Arrays.asList(values)); // should be as records(0)
	        
			int key;
			double value = 0;
			double oldValue;
		        
		    while ((line = br.readLine()) != null) {
		        values = line.split(USE_DELIMITER);
		        records.add(Arrays.asList(values));
		        
				for (int j = 0; j < records.get(1).size();) {
					if(hasData(records.get(1).get(j))) {
						//Find the run first
						String title = records.get(0).get(j);
						int indexOfSharpBeforeRunNumber = title.indexOf("#");
						int indexOfSpaceAfterRunNumber = title.indexOf(" ", indexOfSharpBeforeRunNumber);
						String runNumberString = title.substring(indexOfSharpBeforeRunNumber + 1, indexOfSpaceAfterRunNumber);
						int runNumber = Integer.valueOf(runNumberString);
						
						
						key = (int)(Double.parseDouble(records.get(1).get(j))/cummulateInterval); // The time
						//if((int)(Double.parseDouble(records.get(1).get(j)) % cummulateInterval) == 0)
						//		key --;
						
						try {
							value = Double.parseDouble(records.get(1).get(j+1)) / convertData; // the receiveData at the time
						}
						catch(NumberFormatException e) {
							System.out.println();
						}
						
						TreeMap<Integer, Double> entry = cummulateReceiveDataMapByTime.get(runNumber);
						if(entry == null) {
							entry = new TreeMap<Integer, Double>();
							cummulateReceiveDataMapByTime.put(runNumber, entry);
						}
						
						if(entry.containsKey(key))
							oldValue = entry.get(key);
						else
							oldValue = 0;
							
						entry.put(Integer.valueOf(key), Double.valueOf(value + oldValue));
						j += 2;
					}
					else
						j++;
				}
				
				records.remove(1);
		    }
			    
			System.out.println("Process file done!");
			
			
			int hyphenIndex = filePath.indexOf("-");
			int csvIndex = filePath.indexOf(".csv");
			String hyphenBegin = filePath.substring(0, hyphenIndex);
			String csvEnd = filePath.substring(csvIndex);
			String outputFile = hyphenBegin + "-processed" + csvEnd;
				
			writeTreeMapToCsv(outputFile, cummulateReceiveDataMapByTime);
			
			System.out.println("Write to file " + outputFile + " successfully!");
		}
	    else 
	    	System.out.println("Empty file!");
	}
	
	private static void writeTreeMapToCsv(String outputFile, TreeMap<Integer, TreeMap<Integer, Double>> theTree) {
		try (Writer writer = new FileWriter(outputFile, true)) {
			  for (Entry<Integer, TreeMap<Integer, Double>> outerEntry : theTree.entrySet()) {
				  writer.append("#"+outerEntry.getKey()).append("\n");
				  
				  for(Entry<Integer, Double> innerEntry : outerEntry.getValue().entrySet()) {
						writer.append("" + innerEntry.getKey())
						      .append(USE_DELIMITER)
						      .append(""+ innerEntry.getValue())
						      .append("\n");
				  }
			  }
			} catch (IOException ex) {
			  ex.printStackTrace(System.err);
			}
	}
	
	private static List<String> getRecordFromLine(String line) {
	    List<String> values = new ArrayList<String>();
	    try (Scanner rowScanner = new Scanner(line)) {
	        rowScanner.useDelimiter(USE_DELIMITER);
	        while (rowScanner.hasNext()) {
	            values.add(rowScanner.next());
	        }
	    }
	    return values;
	}	
	
	private static boolean hasData(String content) {
		if(content.equals(""))
			return false;
		return true;
	}
}
