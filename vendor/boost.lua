local boost_path = os.getenv("BOOST_ROOT")

if(not boost_path) then
	print("Boost not found. Make sure to create environment variable 'BOOST_ROOT' with path to it.")
	os.exit(-1)
end

system_files (boost_path) {
	"boost/",
}
