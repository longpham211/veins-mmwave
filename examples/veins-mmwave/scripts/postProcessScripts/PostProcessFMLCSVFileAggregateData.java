import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Map.Entry;
import java.util.TreeMap;

public class PostProcessFMLCSVFileAggregateData {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
//	public static String SAMBA_ALGORITHM = "SAMBA";
//	public static String IEEE80211AD_ALGORITHM = "ieee80211ad";
//	public static String FML_ALGORITHM = "FML";
	
	public static String factor0 = "0.0023";
	public static String factor1 = "0.0045";
	public static String factor2 = "0.0068";
	public static String factor3 = "0.0090";
	public static String factor4 = "0.0113";
	public static String factor5 = "0.0135";
	public static String factor6 = "0.0158";
	public static String factor7 = "0.0180";
	public static String factor8 = "0.0203";
	public static String factor9 = "0.0225";
	
	public static String ALGORITHM_HEADER = "Algorithm";
	public static String TIMESLOT_HEADER= "Timeslot";
	public static String MEAN_AGGREGATE_DATA_HEADER = "Mean";
	public static String SD_AGGREGATE_DATA_HEADER = "SD";
	
	public static void main(String[] args) throws Exception { 
		String inputFilePath = args[0];
		String outputFilePath = args[1];
		
		String algorithm;
		if(inputFilePath.contains("0"))
			algorithm = factor0;
		else if (inputFilePath.contains("1"))
			algorithm = factor1;
		else if (inputFilePath.contains("2"))
			algorithm = factor2;
		else if (inputFilePath.contains("3"))
			algorithm = factor3;
		else if (inputFilePath.contains("4"))
			algorithm = factor4;
		else if (inputFilePath.contains("5"))
			algorithm = factor5;
		else if (inputFilePath.contains("6"))
			algorithm = factor6;
		else if (inputFilePath.contains("7"))
			algorithm = factor7;
		else if (inputFilePath.contains("8"))
			algorithm = factor8;
		else if (inputFilePath.contains("9"))
			algorithm = factor9;
		else
			throw new Exception("Input file is not FML FactorControlFunction scenario");
		
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
