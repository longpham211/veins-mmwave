import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.Writer;
import java.util.ArrayList;

public class PostProcessCSVFileMCS {
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	public static String SAMBA_ALGORITHM = "SAMBA";
	public static String IEEE80211AD_ALGORITHM = "ieee80211ad";
	public static String FML_ALGORITHM = "FML";
	public static String ALGORITHM_HEADER = "Algorithm";
	public static String MCS_HEADER= "MCS";
	public static String COUNT_HEADER = "Count";
	public static String USAGE_HEADER = "ChannelUsage";
	public static String UNUSED_HEADER = "Unused";
	
	public static void main(String[] args) throws Exception { 
		String inputFilePath = args[0];
		String outputFilePath = args[1];
		String outputFilePath2 = args[2];
		String hour = args[3]; // 1/2/3/4/5/.../24
		
		String algorithm;

		int hourValue = Integer.parseInt(hour);

		if(inputFilePath.contains(SAMBA_ALGORITHM))
			algorithm = SAMBA_ALGORITHM;
		else if (inputFilePath.contains(FML_ALGORITHM))
			algorithm = FML_ALGORITHM;
		else if (inputFilePath.contains(IEEE80211AD_ALGORITHM))
			algorithm = IEEE80211AD_ALGORITHM;
		else
			throw new Exception("Input file is not SAMBA, FML, or ieee80211ad");
		
		BufferedReader br = new BufferedReader(new FileReader(inputFilePath));
	    String line;
	    String[] values;
	    int totalRun = 0;
	    double channelUsage = 0;
	    
	    ArrayList<Integer> usedMCS = new ArrayList<Integer>();
	    while ((line = br.readLine()) != null) {
	    	values = line.split(USE_DELIMITER);
	    	if(values[0].contains("#")) { //run line;
	    		totalRun ++;
	    		continue;
	    	}
	    	
	    	int timeSlot = Integer.parseInt(values[0]);
	    	int mcsUsed = Integer.parseInt(values[1]);
	    	
	    	if(timeSlot == (hourValue - 1)) {
	    		usedMCS.add(mcsUsed);
	    	} 
	    }
	    
	    double[] transmissionTimeForMCS = {21.861, 10.932, 8.746, 7.289, 6.728, 5.467, 4.374, 3.645, 3.365, 2.735, 2.188, 1.824}; //Starting from MCS1
			    
	    System.out.println("Process file done!");
	    
	    
	    File outputFile = new File(outputFilePath);
	    Writer writer = null;

	    File outputFile2 = new File(outputFilePath2);
	    Writer writer2 = null;

	    try {
		    if(!outputFile.exists()) {
		    	System.out.println("File does not exist, create file and add header!");
		    	writer = new FileWriter(outputFile, true);
		    	writer.append(ALGORITHM_HEADER)
		    		  .append(USE_DELIMITER)
		    		  .append(MCS_HEADER)
		    		  .append(USE_DELIMITER)
		    		  .append(COUNT_HEADER)
		    		  .append("\n");
		    }
		    else {
		    	writer = new FileWriter(outputFile, true);
		    }

		    if(!outputFile2.exists()) {
		    	System.out.println("File does not exist, create file and add header!");
		    	writer2 = new FileWriter(outputFile2, true);
		    	writer2.append(ALGORITHM_HEADER)
		    		  .append(USE_DELIMITER)
		    		  .append("Type")
		    		  .append(USE_DELIMITER)
		    		  .append("Proportion")
		    		  .append("\n");
		    }
		    else {
		    	writer2 = new FileWriter(outputFile2, true);
		    }

		    assert(usedMCS.size() == 13);
		    
		    for(int i = 0 ; i < transmissionTimeForMCS.length; i++)
		    	channelUsage += transmissionTimeForMCS[i] * usedMCS.get(i+1);
		    			
		    channelUsage = channelUsage/(3600*1000)*100; //%
		    channelUsage = Math.round(channelUsage * 100.0)/100.0;
		    double unused = Math.round((100 - channelUsage) * 100.0)/100.0;

		    for(int i = 1 ; i < usedMCS.size(); i++) {
			    
		    	writer.append(algorithm)
	    		  .append(USE_DELIMITER)
		          .append("MCS " + i)
		          .append(USE_DELIMITER)
		          .append(usedMCS.get(i) +"")
		          .append("\n");
			
		    }
		writer2.append(algorithm)
		  .append(USE_DELIMITER)
		  .append("Transmission")
		  .append(USE_DELIMITER)
		  .append(channelUsage + "")
		  .append("\n");

		writer2.append(algorithm)
		  .append(USE_DELIMITER)
		  .append("Unused and overhead")
		  .append(USE_DELIMITER)
		  .append(unused + "")
		  .append("\n");
	    }
	    finally {
	    	writer.close();
		writer2.close();
	    }
	    
	    System.out.println("Write file successfully!");
	}
}
