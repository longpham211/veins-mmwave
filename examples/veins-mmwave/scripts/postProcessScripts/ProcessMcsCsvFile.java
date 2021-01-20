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


public class ProcessMcsCsvFile {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	public static double GIGA = 1e9;
	public static double MEGA = 1e6;
	public static double KILO = 1e3;
	
	public static void main(String[] args) throws FileNotFoundException, IOException {
//		String filePath = args[0];
//		int cummulateInterval = Integer.parseInt(args[1]); 
//		String convertDataString = args[2];		
		String filePath = args[0];
		int cummulateInterval = 3600;
		
//		double convertData = GIGA;
//		if(convertDataString.equals("K"))
//			convertData = KILO;
//		else if(convertDataString.equals("M"))
//			convertData = MEGA;
//		else if(convertDataString.equals("G"))
//			convertData = GIGA;

		TreeMap<Integer, TreeMap<Integer, int[]>> cummulateReceiveDataMapByTime = new TreeMap<>(); 
		
		List<List<String>> records = new ArrayList<>();
		BufferedReader br = new BufferedReader(new FileReader(filePath));
	    String line;
	    line = br.readLine(); // read the first line
	    if(line != null) {
	        String[] values = line.split(USE_DELIMITER);
	        records.add(Arrays.asList(values)); // should be as records(0)
	        
			int key;
			int value = 0;
			int[] oldValue;
			int mcs = 0; //line 2 = 0
			int lineNumber = 0; // line 2 = 0;
		        
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
							value = Integer.parseInt(records.get(1).get(j+1)); // MCS
						}
						catch(NumberFormatException e) {
							System.out.println();
						}
						
						TreeMap<Integer, int[]> entryRunNumber = cummulateReceiveDataMapByTime.get(runNumber);
						if(entryRunNumber == null) {
							entryRunNumber = new TreeMap<Integer, int[]>();
							cummulateReceiveDataMapByTime.put(runNumber, entryRunNumber);
						}
						
						if(entryRunNumber.containsKey(key)) { //new timeslot
							oldValue = entryRunNumber.get(key);
						}
						else {
							oldValue = new int[13];
						}
						
						mcs = lineNumber % 13;
						oldValue[mcs] += value;
						
						entryRunNumber.put(Integer.valueOf(key), oldValue);
						j += 2;
					}
					else
						j++;
				}
				
				records.remove(1);
				lineNumber ++; 
				
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
	
	private static void writeTreeMapToCsv(String outputFile, TreeMap<Integer, TreeMap<Integer, int[]>> theTree) {
		try (Writer writer = new FileWriter(outputFile, true)) {
			  for (Entry<Integer, TreeMap<Integer, int[]>> outerEntry : theTree.entrySet()) {
				writer.append("#"+outerEntry.getKey()).append("\n");
				  
				for(Entry<Integer, int[]> innerEntry : outerEntry.getValue().entrySet()) {
				    int[] mcsUsed = innerEntry.getValue();
				  
				    for(int i = 0; i < mcsUsed.length; i ++)
						writer.append("" + innerEntry.getKey())
						      .append(USE_DELIMITER)
						      .append(""+ mcsUsed[i])
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
