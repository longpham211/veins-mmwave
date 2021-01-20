import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Map.Entry;
import java.util.TreeMap;

public class PostProcessWithOrWithoutTLSCSVFileAggregateData {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	public static String SAMBA_ALGORITHM = "SAMBA";
	public static String IEEE80211AD_ALGORITHM = "ieee80211ad";
	public static String FML_ALGORITHM = "FML";
	
	public static String withoutKeyWord = "Without";
	
	public static String ALGORITHM_HEADER = "Algorithm";
	public static String TIMESLOT_HEADER= "Timeslot";
	public static String MEAN_AGGREGATE_DATA_HEADER = "Mean";
	public static String SD_AGGREGATE_DATA_HEADER = "SD";
	
	public static void main(String[] args) throws Exception { 
		String inputFilePath = args[0];
		String outputFilePath = args[1];
		
		String algorithm;
		if(inputFilePath.contains(IEEE80211AD_ALGORITHM))
			algorithm = IEEE80211AD_ALGORITHM;
		else if (inputFilePath.contains(SAMBA_ALGORITHM))
			algorithm = SAMBA_ALGORITHM;
		else if (inputFilePath.contains(FML_ALGORITHM))
			algorithm = FML_ALGORITHM;
		else
			throw new Exception("Input file does not contain FML, ieee80211ad, or SAMBA keyword");

		if(inputFilePath.contains(withoutKeyWord))
			algorithm += " Without TLS";
		else
			algorithm += " With TLS";
		
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
			          .append("" + entry)
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