import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Map.Entry;
import java.util.TreeMap;

public class PostProcessIEEE80211ADCSVFileAggregateData {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
//	public static String SAMBA_ALGORITHM = "SAMBA";
//	public static String IEEE80211AD_ALGORITHM = "ieee80211ad";
//	public static String FML_ALGORITHM = "FML";
	
	public static String bi30 = "30ms";
	public static String bi40 = "40ms";
	public static String bi50 = "50ms";
	public static String bi60 = "60ms";
	public static String bi70 = "70ms";
	public static String bi80 = "80ms";
	public static String bi90 = "90ms";
	public static String bi100 = "100ms";
	
	public static String ALGORITHM_HEADER = "Algorithm";
	public static String TIMESLOT_HEADER= "Timeslot";
	public static String MEAN_AGGREGATE_DATA_HEADER = "Mean";
	public static String SD_AGGREGATE_DATA_HEADER = "SD";
	
	public static void main(String[] args) throws Exception { 
		String inputFilePath = args[0];
		String outputFilePath = args[1];
		
		String[] hours = {"12 a.m.",
		"1 a.m.",
		"2 a.m.",
		"3 a.m.",
		"4 a.m.",
		"5 a.m.",
		"6 a.m.",
		"7 a.m.",
		"8 a.m.",
		"9 a.m.",
		"10 a.m.",
		"11 a.m.",
		"12 p.m.", 
		"1 p.m.",
		"2 p.m.",
		"3 p.m.",
		"4 p.m.",
		"5 p.m.",
		"6 p.m.",
		"7 p.m.",
		"8 p.m.",
		"9 p.m.",
		"10 p.m.",
		"11 p.m.",
		"12 a.m."};

		String algorithm;
		if(inputFilePath.contains("30"))
			algorithm = bi30;
		else if (inputFilePath.contains("40"))
			algorithm = bi40;
		else if (inputFilePath.contains("50"))
			algorithm = bi50;
		else if (inputFilePath.contains("60"))
			algorithm = bi60;
		else if (inputFilePath.contains("70"))
			algorithm = bi70;
		else if (inputFilePath.contains("BI80"))
			algorithm = bi80;
		else if (inputFilePath.contains("90"))
			algorithm = bi90;
		else if (inputFilePath.contains("100"))
			algorithm = bi100;
		else
			throw new Exception("Input file is not IEEEE80211ad Beacon Interval study scenario");
		
		BufferedReader br = new BufferedReader(new FileReader(inputFilePath));
	    String line;
	    String[] values;
	    int totalRun = 0;
	    TreeMap<Integer, ArrayList<Double>> totalAggregateDataPerTimeSlot = new TreeMap<>();
	    
	    while ((line = br.readLine()) != null) {
	    	values = line.split(USE_DELIMITER);
	    	if(values[0].contains("#")) { //run line;
	    		totalRun ++;
	    		continue;
	    	}
	    	
	    	int timeSlot = Integer.parseInt(values[0]);
	    	double aggregateData = Double.parseDouble(values[1]);
	 
	    	ArrayList<Double> entry = totalAggregateDataPerTimeSlot.get(timeSlot);
	    	if(entry == null) {
	    		entry = new ArrayList<Double>();
	    		totalAggregateDataPerTimeSlot.put(timeSlot, entry);
	    	}
    		entry.add(aggregateData);
	    }
		
	    int numberOfTimeSlots = totalAggregateDataPerTimeSlot.lastKey() + 1;
	    
	    double[] means = new double[numberOfTimeSlots];
	    double[] sd = new double[numberOfTimeSlots];
	    
	    for(Entry<Integer, ArrayList<Double>> entry : totalAggregateDataPerTimeSlot.entrySet()) {
	    	assert totalRun == entry.getValue().size();
	    	
	    	double sum = 0;
	    	double sumVariance = 0;
	    	
	    	for(int i = 0 ; i < entry.getValue().size(); i ++)
	    		sum += entry.getValue().get(i);
	    	
	    	double meanValue = sum / totalRun;
	    	means[entry.getKey()] = meanValue;
	    	
	    	for(int i = 0 ; i < entry.getValue().size(); i ++)
	    		sumVariance += Math.pow(entry.getValue().get(i) - meanValue, 2);
	    	
	    	double variance = sumVariance / totalRun;
	    	sd[entry.getKey()] = Math.sqrt(variance);
	    }
	    
	    System.out.println("Process file done!");
	    
	    
	    File outputFile = new File(outputFilePath);
	    Writer writer = null;
	    try {
		    if(!outputFile.exists()) {
	//	    	outputFile.createNewFile();
		    	System.out.println("File does not exist, create file and add header!");
		    	writer = new FileWriter(outputFile, true);
		    	writer.append(ALGORITHM_HEADER)
		    		  .append(USE_DELIMITER)
		    		  .append(TIMESLOT_HEADER)
		    		  .append(USE_DELIMITER)
		    		  .append(MEAN_AGGREGATE_DATA_HEADER)
		    		  .append(USE_DELIMITER)
		    		  .append(SD_AGGREGATE_DATA_HEADER)
		    		  .append("\n");
		    }
		    else {
		    	writer = new FileWriter(outputFile, true);
		    }
		    
		    for(Integer entry : totalAggregateDataPerTimeSlot.keySet()) {	    	
		    	writer.append(algorithm)
		    		  .append(USE_DELIMITER)
			          .append(hours[entry + 1])
			          .append(USE_DELIMITER)
			          .append("" + means[entry])
			          .append(USE_DELIMITER)
			          .append("" + sd[entry])
			          .append("\n");
		    }		    
	    }
	    finally {
	    	writer.close();
	    }
	    
	    System.out.println("Write file successfully!");
	}
}
