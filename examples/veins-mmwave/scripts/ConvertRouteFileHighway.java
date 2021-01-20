import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class ConvertRouteFileHighway {
	public static String Datum = "Datum";
	public static String Stunde = "Stunde";
	
	public static String Pkw_R1 = "Pkw_R1";
	public static String Lfw_R1 = "Lfw_R1";
	public static String Mot_R1 = "Mot_R1";
	public static String PmA_R1 = "PmA_R1";
	public static String Bus_R1 = "Bus_R1";
	public static String LoA_R1 = "LoA_R1";
	public static String Lzg_R1 = "Lzg_R1";
	public static String Sat_R1 = "Sat_R1";
	public static String Son_R1 = "Son_R1";

	public static String Pkw_R2 = "Pkw_R2";
	public static String Lfw_R2 = "Lfw_R2";
	public static String Mot_R2 = "Mot_R2";
	public static String PmA_R2 = "PmA_R2";
	public static String Bus_R2 = "Bus_R2";
	public static String LoA_R2 = "LoA_R2";
	public static String Lzg_R2 = "Lzg_R2";
	public static String Sat_R2 = "Sat_R2";
	public static String Son_R2 = "Son_R2";
	
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	
	public static void main (String[] args) throws IOException {
		String inputFilePath = "/home/longpham211/Downloads/zst6826/zst6826_2018.csv";
		String outputFilePath = "/home/longpham211/Downloads/zst6826/routeHighway.txt";
		
		int Datum_index = 0;
		int Stunde_index = 0;
		
		int Pkw_R1_index = 0;
		int Lfw_R1_index = 0;
		int Mot_R1_index = 0;
		int PmA_R1_index = 0;
		int Bus_R1_index = 0;
		int LoA_R1_index = 0;
		int Lzg_R1_index = 0;
		int Sat_R1_index = 0;
		int Son_R1_index = 0;

		int Pkw_R2_index = 0;
		int Lfw_R2_index = 0;
		int Mot_R2_index = 0;
		int PmA_R2_index = 0;
		int Bus_R2_index = 0;
		int LoA_R2_index = 0;
		int Lzg_R2_index = 0;
		int Sat_R2_index = 0;
		int Son_R2_index = 0;		
		
	    File outputFile = new File(outputFilePath);
	    Writer writer = new FileWriter(outputFile, true);
	    
		List<List<String>> records = new ArrayList<>();
		
		BufferedReader br = new BufferedReader(new FileReader(inputFilePath));
	    String line;
	    line = br.readLine(); // read the first line
	    
        String[] values = line.split(USE_DELIMITER);
        records.add(Arrays.asList(values)); // should be as records(0)
        
        for(int i = 0 ; i < values.length ; i++) {
        	if(values[i].equals(Datum))
        		Datum_index = i;
        	else if(values[i].equals(Stunde))
        		Stunde_index = i;
        	else if(values[i].equals(Pkw_R1))
        		Pkw_R1_index = i;
        	else if(values[i].equals(Lfw_R1))
        		Lfw_R1_index = i;
        	else if(values[i].equals(Mot_R1))
        		Mot_R1_index = i;
        	else if(values[i].equals(PmA_R1))
        		PmA_R1_index = i;
        	else if(values[i].equals(Bus_R1))
        		Bus_R1_index = i;
        	else if(values[i].equals(LoA_R1))
        		LoA_R1_index = i;
        	else if(values[i].equals(Lzg_R1))
        		Lzg_R1_index = i;
        	else if(values[i].equals(Sat_R1))
        		Sat_R1_index = i;
        	else if(values[i].equals(Son_R1))
        		Son_R1_index = i;
        	else if(values[i].equals(Pkw_R2))
        		Pkw_R2_index = i;
        	else if(values[i].equals(Lfw_R2))
        		Lfw_R2_index = i;
        	else if(values[i].equals(Mot_R2))
        		Mot_R2_index = i;
        	else if(values[i].equals(PmA_R2))
        		PmA_R2_index = i;
        	else if(values[i].equals(Bus_R2))
        		Bus_R2_index = i;
        	else if(values[i].equals(LoA_R2))
        		LoA_R2_index = i;
        	else if(values[i].equals(Lzg_R2))
        		Lzg_R2_index = i;
        	else if(values[i].equals(Sat_R2))
        		Sat_R2_index = i;
        	else if(values[i].equals(Son_R2))
        		Son_R2_index = i;
        }
        
        String[] vehicleTypes = {"pkw_ohne_anhaenger",
        					     "lieferwagen_ohne_anhaenger",
        					     "motorraedder",
        					     "pkw_mit_anhaenger",
        					     "busse",
        					     "lkw_ohne_anhaenger",
        					     "sattelkraftfahrzeuge",
        					     "lkw_mit_anhaenger",
        					     "nicht_klassifizierbare_Kfz"};
        int[] vehsPerHourR1Indexes = {Pkw_R1_index, 
        						     Lfw_R1_index, 
        						     Mot_R1_index, 
        						     PmA_R1_index,
        						     Bus_R1_index,
        						     LoA_R1_index,
        						     Sat_R1_index,
        						     Lzg_R1_index,
        						     Son_R1_index};
        int[] vehsPerHourR2Indexes = {Pkw_R2_index, 
			     					  Lfw_R2_index, 
			     					  Mot_R2_index, 
			     					  PmA_R2_index,
			     					  Bus_R2_index,
			     					  LoA_R2_index,
			     					  Sat_R2_index,
			     					  Lzg_R2_index,
			     					  Son_R2_index};
     
	    while ((line = br.readLine()) != null) {
	        values = line.split(USE_DELIMITER);
	        int baseHourValue = 0;
	        if(!(values[Datum_index].equalsIgnoreCase("180804") || values[Datum_index].equalsIgnoreCase("180805") || values[Datum_index].equalsIgnoreCase("180806")))
	        	continue;
	        else if (values[Datum_index].equalsIgnoreCase("180805")) {
	        	baseHourValue = 24;
	        }
	        else if (values[Datum_index].equalsIgnoreCase("180806")) {
	        	baseHourValue = 48;
	        }
	        	
	        
	        int hourValue = baseHourValue + Integer.valueOf(values[Stunde_index]);
	        
	        for(int i = 0 ; i < 9; i++) {
	        	double beginValue = (hourValue - 1) * 3600 + (0.07 * (i+1));
	        	double endValue = (hourValue) * 3600 + (0.07 * (i+1));
	        	beginValue = (double) Math.round(beginValue * 100) / 100;
	        	endValue = (double) Math.round(endValue * 100) / 100;
	        	int vehsPerHourValue = Integer.valueOf(removeWhiteSpaces(values[vehsPerHourR1Indexes[i]]));
	        	if(i == 7) // lkw_mit_anhaenger
	        		vehsPerHourValue = Integer.valueOf(removeWhiteSpaces(values[vehsPerHourR1Indexes[i]])) - Integer.valueOf(removeWhiteSpaces(values[vehsPerHourR1Indexes[i-1]]));
	        	
	        	if(vehsPerHourValue == 0)
	        		continue;
	        	
	        	//Take value of lzg minus value of sat
	        	String outPutLine = "<flow id=\"s2n_r" 
	        						+ (i+1) 
	        						+ "_" 
	        						+ values[Datum_index]
	        						+ "_"
	        						+ values[Stunde_index]
	        						+ "\" begin=\""  
	        						+ beginValue
	        						+"\" end=\""
	        						+ endValue
	        						+"\" vehsPerHour=\""
	        						+ vehsPerHourValue
	        						+"\" type=\""
	        						+ vehicleTypes[i]
	        						+"\" from=\"234004934\" to=\"51219147\"/>";
	        			writer.append(TAB_DELIMITER)
	        				  .append(outPutLine)
	        				  .append("\n");
	        }
	        
	        for(int i = 0 ; i < 9; i++) {
	        	double beginValue = (hourValue - 1) * 3600 + (0.07 * (i+10));
	        	double endValue = (hourValue) * 3600 + (0.07 * (i+10));
	        	beginValue = (double) Math.round(beginValue * 100) / 100;
	        	endValue = (double) Math.round(endValue * 100) / 100;
	        	int vehsPerHourValue = Integer.valueOf(removeWhiteSpaces(values[vehsPerHourR2Indexes[i]]));
	        	if(i == 7) // lkw_mit_anhaenger
	        		vehsPerHourValue = Integer.valueOf(removeWhiteSpaces(values[vehsPerHourR2Indexes[i]])) - Integer.valueOf(removeWhiteSpaces(values[vehsPerHourR2Indexes[i-1]]));

	        	if(vehsPerHourValue == 0)
	        		continue;
	        	
	        	//Take value of lzg minus value of sat
	        	String outPutLine = "<flow id=\"n2s_r" 
	        						+ (i+1) 
	        						+ "_" 
	        						+ values[Datum_index]
	        						+ "_"
	        						+ values[Stunde_index]
	        						+ "\" begin=\""  
	        						+ beginValue
	        						+"\" end=\""
	        						+ endValue
	        						+"\" vehsPerHour=\""
	        						+ vehsPerHourValue
	        						+"\" type=\""
	        						+ vehicleTypes[i]
	        						+"\" from=\"284791345\" to=\"388248005\"/>";
	        			writer.append(TAB_DELIMITER)
	        				  .append(outPutLine)
	        				  .append("\n");
	        }
	        
	        writer.append("\n\n");
	    }
	    writer.close();
	    System.out.println("Done!");
	}
	
	public static String removeWhiteSpaces(String valueString) {
		return valueString.replaceAll("\\s+","");
	}
}
